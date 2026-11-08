#!/usr/local/bin/bash
#
# 把目录树标记为“低完整性”(Low)，让运行在低完整性级别的沙箱进程可以写它。
#
# 背景：Windows 的强制完整性控制禁止低完整性进程写中等完整性对象，这与 ACL 无关，
# 所以只补权限项是没用的，必须改完整性标签。DSH 之类的沙箱只给工作区根目录打标签，
# 已存在的子目录不会回溯继承，于是出现“根目录能写、src/out 写不了”的现象。
#
# 用法：
#   ./script/sandbox_label.sh apply    [-p <目录>]   # 打 Low 标签，沙箱内可写
#   ./script/sandbox_label.sh restore  [-p <目录>]   # 恢复 Medium，沙箱内只读
#   ./script/sandbox_label.sh status   [-p <目录>]   # 只读检查当前标签
#   ./script/sandbox_label.sh dry-run  [-p <目录>]   # 只统计对象数量
#
# 注意：
#   * 必须在沙箱之外运行（普通终端），沙箱内的低完整性进程无法修改安全标签；
#     msys2 的 bash 在 DSH 沙箱里甚至连信号管道都建不起来。
#   * 以目录所有者的身份运行即可，不需要管理员：所有者有隐含的改权限能力，
#     脚本会先补一条“取得所有权”(WO)权限项，这是设置标签的前提。
#   * 只改完整性标签和权限项，不动文件内容和所有者。

# 阻止 MSYS/Git-Bash 把 "/grant" 这类参数当成路径转换掉
export MSYS2_ARG_CONV_EXCL='*'
export MSYS_NO_PATHCONV=1

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

DEFAULT_TARGET="${ROOT_DIR}"

function usage() {
    cat <<EOF
用法: $(basename "$0") <apply|restore|status|dry-run> [-p <目录>]

  apply     把目录树标记为低完整性(Low)，沙箱进程可写
  restore   恢复为中等完整性(Medium)，沙箱进程回到只读
  status    只读检查：打印目录及其子项的完整性标签
  dry-run   只统计将要处理的对象数量，不做任何改动

  -p, --path <目录>   目标目录，默认仓库根目录: ${DEFAULT_TARGET}
EOF
}

function to_win_path() {
    local path="${1}"
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -w "${path}"
    else
        printf '%s' "${path}"
    fi
}

function current_account() {
    if [ -n "${USERDOMAIN}" ] && [ -n "${USERNAME}" ]; then
        printf '%s\\%s' "${USERDOMAIN}" "${USERNAME}"
    else
        whoami.exe 2>/dev/null | tr -d '\r' | tr '/' '\\'
    fi
}

function count_objects() {
    find "${1}" 2>/dev/null | wc -l | tr -d ' '
}

function run_icacls() {
    local desc="${1}"
    shift
    local output
    output="$(icacls.exe "$@" 2>&1)"
    local code=$?
    if [ ${code} -ne 0 ]; then
        print_log "${desc} 失败，退出码 ${code}" error
        printf '%s\n' "${output}" | tail -n 5
        return 1
    fi
    print_log "${desc} 成功" info
    return 0
}

function grant_write_owner() {
    local win_path="${1}"
    local account="${2}"
    run_icacls "授予 ${account} 取得所有权权限" "${win_path}" /grant "${account}:(WO)" /T /C /Q
}

function set_integrity_level() {
    local win_path="${1}"
    local level="${2}"
    run_icacls "设置完整性标签 ${level}" "${win_path}" /setintegritylevel "(OI)(CI)${level}" /T /C /Q
}

function show_status() {
    local win_path="${1}"
    local total
    total="$(count_objects "${win_path}")"
    print_log "目标 ${win_path}，共 ${total} 个对象，标签行如下：" info
    # icacls 的标签行带 (NW) 策略标记，不受系统语言影响
    icacls.exe "${win_path}" 2>/dev/null | grep -F 'NW' | head -n 1
    local sample
    sample="$(find "${win_path}" -mindepth 1 -maxdepth 2 -type d 2>/dev/null | head -n 3)"
    while IFS= read -r dir; do
        [ -z "${dir}" ] && continue
        local line
        line="$(icacls.exe "$(to_win_path "${dir}")" 2>/dev/null | grep -F 'NW' | head -n 1)"
        [ -n "${line}" ] && printf '%s\n' "${line}"
    done <<<"${sample}"
}

function main() {
    local command="${1:-}"
    shift 2>/dev/null || true

    local target="${DEFAULT_TARGET}"
    while [ $# -gt 0 ]; do
        case "${1}" in
        -p | --path)
            target="${2:-}"
            shift 2
            ;;
        -h | --help)
            usage
            exit 0
            ;;
        *)
            print_log "未知参数: ${1}" error
            usage
            exit 1
            ;;
        esac
    done

    if [ "${command}" == "-h" ] || [ "${command}" == "--help" ]; then
        usage
        exit 0
    fi
    if [ -z "${command}" ]; then
        usage
        exit 1
    fi

    case "$(uname -s)" in
    MINGW* | MSYS* | CYGWIN*) ;;
    *)
        print_log "只支持 Windows（需要 icacls.exe 和完整性级别）" error
        exit 2
        ;;
    esac

    if [ ! -d "${target}" ]; then
        print_log "目录不存在: ${target}" error
        exit 2
    fi

    local win_path
    win_path="$(to_win_path "${target}")"
    if [[ "${win_path}" =~ ^[A-Za-z]:\\?$ ]]; then
        print_log "拒绝处理盘符根目录: ${win_path}" error
        exit 2
    fi

    local total
    total="$(count_objects "${target}")"

    case "${command}" in
    apply | restore)
        local level="L"
        [ "${command}" == "restore" ] && level="M"
        local account
        account="$(current_account)"
        print_log "开始 ${command}: ${win_path}（${total} 个对象），账号 ${account}" info
        local begin=${SECONDS}
        grant_write_owner "${win_path}" "${account}" || exit 3
        set_integrity_level "${win_path}" "${level}" || exit 3
        print_log "${command} 完成，耗时 $((SECONDS - begin)) 秒" info
        show_status "${win_path}"
        ;;
    status)
        show_status "${win_path}"
        ;;
    dry-run)
        print_log "[dry-run] 将处理 ${total} 个对象: ${win_path}" info
        print_log "[dry-run] 步骤: 授予取得所有权权限 -> 设置完整性标签 -> 打印结果标签" info
        ;;
    *)
        print_log "未知命令: ${command}" error
        usage
        exit 1
        ;;
    esac
    return 0
}

main "$@"
