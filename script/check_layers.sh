#!/usr/bin/env bash
#
# 检查模块分层：只允许“高层依赖低层”，禁止低层反向 include 高层。
#
#    common  <-  container
#            <-  utils
#    logging <-  common / container / utils
#
# 只看项目内绝对形式的 include（"origin/<module>/..."）；同目录相对 include
# （"internal/..."）不参与判断。src/tests 是聚合测试入口，不做限制。
# 该检查不依赖编译数据库，几秒内就能跑完，CI 和提交前都可以用。
#
# 用法：
#   ./script/check_layers.sh

SCRIPT_DIR="$(
    cd "$(dirname "$0")" || exit 1
    pwd
)"
ROOT_DIR="$(
    cd "${SCRIPT_DIR}/.." || exit 1
    pwd
)"
# shellcheck disable=SC1091
source "${SCRIPT_DIR}/common_func.sh"

cd "${ROOT_DIR}" || exit 1

function usage() {
    cat <<EOF
用法: $(basename "$0")

检查模块分层，禁止低层 include 高层：
  common <- container
         <- utils
  logging <- common / container / utils
EOF
}

if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    usage
    exit 0
fi

# bash 3.2（macOS 自带）没有关联数组，用 case 代替。
function module_rank() {
    case "$1" in
    common) echo 0 ;;
    container) echo 1 ;;
    utils) echo 1 ;;
    logging) echo 2 ;;
    *) echo 99 ;;
    esac
}

# 项目 include 的路径前缀和所属库并不总是一一对应：
#   utils 库对外暴露 origin/{utils,filesystem,date_time,process,thread,strings}/
#   common 库还包含生成的 origin/version.h、origin/build_info.h
function module_of_include() {
    case "$1" in
    origin/common/* | origin/version.h | origin/build_info.h) echo common ;;
    origin/container/*) echo container ;;
    origin/logging/*) echo logging ;;
    origin/utils/* | origin/filesystem/* | origin/date_time/* | origin/process/* | \
        origin/thread/* | origin/strings/*) echo utils ;;
    *) echo unknown ;;
    esac
}

VIOLATIONS=0

while IFS= read -r file; do
    # 只看 src/libs/<module>/ 下的文件；src/tests 等聚合入口不限制。
    case "${file}" in
    src/libs/*) ;;
    *) continue ;;
    esac
    module="${file#src/libs/}"
    module="${module%%/*}"
    from_rank="$(module_rank "${module}")"

    while IFS= read -r include_path; do
        to_module="$(module_of_include "${include_path}")"
        if [ "${to_module}" = "unknown" ]; then
            continue
        fi
        to_rank="$(module_rank "${to_module}")"
        if [ "${to_rank}" -gt "${from_rank}" ]; then
            print_log "分层违规: ${file} (${module}) -> ${to_module} (${include_path})" error
            VIOLATIONS=$((VIOLATIONS + 1))
        fi
    done < <(sed -n 's/^#include[[:space:]]*"\(origin\/[^"]*\)".*/\1/p' "${file}")
done < <(
    find src -type f \( \
        -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o \
        -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' \
        \) | sort
)

if [ "${VIOLATIONS}" -gt 0 ]; then
    print_log "发现 ${VIOLATIONS} 处模块分层违规" error
    exit 1
fi

print_log "模块分层检查通过" info
