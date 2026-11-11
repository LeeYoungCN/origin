#!/usr/local/bin/bash
#
# 用 clang 编译一遍并跑测试，提前发现只在 clang 下才会报的错。
# 典型场景：MSVC 默认把"临时对象绑定到非 const 引用"当成扩展放过，clang 直接报错。
#
# 用法：
#   ./script/check_clang.sh                    # 当前平台默认 debug preset：编译 + ctest
#   ./script/check_clang.sh --no-test          # 只编译
#   ./script/check_clang.sh linux_clang_debug  # 指定其它 preset
#
# 默认 preset：Windows=win_clang_cl_debug / Linux=linux_clang_debug / macOS=darwin_clang_debug
# 对应 CI：.github/workflows/cmake-multi-cmake-presets.yml 里 Windows 的 win_clang_cl_debug 行。

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

function usage() {
    cat <<EOF
用法: $(basename "$0") [--no-test] [preset]

  --no-test   只编译，不跑 ctest
  preset      指定 CMake configure preset，默认按平台选择：
              Windows=win_clang_cl_debug / Linux=linux_clang_debug / macOS=darwin_clang_debug
EOF
}

PRESET=""
RUN_TEST=1
for ARG in "$@"; do
    case "${ARG}" in
    --no-test)
        RUN_TEST=0
        ;;
    -h | --help)
        usage
        exit 0
        ;;
    -*)
        print_log "未知参数: ${ARG}" error
        usage
        exit 1
        ;;
    *)
        PRESET="${ARG}"
        ;;
    esac
done

if [ -z "${PRESET}" ]; then
    case "$(uname -s)" in
    MINGW* | MSYS* | CYGWIN*)
        PRESET="win_clang_cl_debug"
        # clang-cl 需要 MSVC 的头文件与库路径
        # shellcheck disable=SC1091
        source "${SCRIPT_DIR}/load_msvc_env.sh"
        ;;
    Linux)
        PRESET="linux_clang_debug"
        ;;
    Darwin)
        PRESET="darwin_clang_debug"
        ;;
    *)
        print_log "不支持的平台: $(uname -s)" error
        exit 1
        ;;
    esac
fi

cd "${ROOT_DIR}" || exit 1

print_log "clang 检查开始: preset=${PRESET}" info

if ! cmake --preset="${PRESET}"; then
    print_log "[${PRESET}] configure failed" error
    exit 1
fi

if ! cmake --build --preset="${PRESET}_build"; then
    print_log "[${PRESET}] build failed" error
    exit 1
fi

if [ "${RUN_TEST}" -eq 1 ]; then
    # 不用 ctest --preset：preset 里的输出路径是相对 CWD 的，而 ctest --preset 又必须在
    # 仓库根执行，会在根目录留下 Testing/。直接进构建目录跑普通 ctest 更干净。
    if ! (
        cd "out/build/${PRESET}" || exit 1
        ctest --timeout 60 --output-on-failure
    ); then
        print_log "[${PRESET}] ctest failed" error
        exit 1
    fi
fi

print_log "clang 检查通过: preset=${PRESET}" info
