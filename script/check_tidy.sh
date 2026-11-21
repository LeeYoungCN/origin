#!/usr/bin/env bash
#
# 用 .clang-tidy 做静态检查（与 clangd 共用同一份配置），提交前手动跑一遍。
# 默认只检查相对 HEAD 的改动文件；--all 检查编译数据库里的全部源文件。
#
# 用法：
#   ./script/check_tidy.sh                 # 只查改动文件（staged + unstaged）
#   ./script/check_tidy.sh --all           # 全量检查（CI 用）
#   ./script/check_tidy.sh --fix           # 尝试自动修复（改完记得重新编译验证）
#   ./script/check_tidy.sh --no-werror     # 只报告告警，不因此失败
#   ./script/check_tidy.sh --github-annotations  # 把告警转成 GitHub 注解（CI 用）
#   ./script/check_tidy.sh --no-configure  # 跳过 configure，复用已有构建目录（CI 用）
#   ./script/check_tidy.sh --prepare-db    # 只准备编译数据库（给 clangd 用），不检查
#   ./script/check_tidy.sh --preset win_clang_cl_debug
#
# 默认 preset：Windows=win_clang_cl_debug / Linux=linux_clang_debug / macOS=darwin_clang_debug
# 对应 CI：.github/workflows/cmake-multi-cmake-presets.yml 的 clang-tidy job。

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

CLANG_TIDY_BIN="${CLANG_TIDY:-clang-tidy}"

function usage() {
    cat <<EOF
用法: $(basename "$0") [--all] [--fix] [--no-werror] [--no-configure] [--preset <name>]

  --all           检查编译数据库中的全部源文件（默认只查改动文件）
  --fix           应用 clang-tidy 修复（默认只报告）
  --no-werror     告警不导致失败（默认 --warnings-as-errors='*'）
  --github-annotations
                  把 clang-tidy 的告警/错误转成 GitHub 注解（CI 用）
  --no-configure  跳过 configure，直接使用已有构建目录（CI 用）
  --prepare-db    只准备编译数据库（复制到 out/build），不执行检查
  --preset <name> 指定 CMake configure preset，默认按平台选择：
                  Windows=win_clang_cl_debug / Linux=linux_clang_debug / macOS=darwin_clang_debug
EOF
}

ALL=0
APPLY_FIX=0
WERROR=1
NO_CONFIGURE=0
PREPARE_DB=0
GITHUB_ANNOTATIONS=0
PRESET=""
PRESET_GIVEN=0

while [ $# -gt 0 ]; do
    case "$1" in
    --all)
        ALL=1
        ;;
    --fix)
        APPLY_FIX=1
        ;;
    --no-werror)
        WERROR=0
        ;;
    --no-configure)
        NO_CONFIGURE=1
        ;;
    --prepare-db)
        PREPARE_DB=1
        ;;
    --github-annotations)
        GITHUB_ANNOTATIONS=1
        ;;
    --preset)
        if [ -z "$2" ]; then
            print_log "--preset 需要参数" error
            exit 1
        fi
        PRESET="$2"
        PRESET_GIVEN=1
        shift
        ;;
    -h | --help)
        usage
        exit 0
        ;;
    *)
        print_log "未知参数: $1" error
        usage
        exit 1
        ;;
    esac
    shift
done

if ! command -v "${CLANG_TIDY_BIN}" >/dev/null 2>&1; then
    print_log "找不到 ${CLANG_TIDY_BIN}，请先安装 LLVM/Clang 或设置 CLANG_TIDY 环境变量" error
    exit 1
fi

if [ "${PRESET_GIVEN}" -eq 0 ]; then
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

DB_DIR="out/build/${PRESET}"
DB_FILE="${DB_DIR}/compile_commands.json"
# clangd 的 .clangd 固定指向 out/build，这里把当前 preset 的数据库复制过去，
# 保证编辑器和脚本看到的是同一份编译参数。
TOP_DB="out/build/compile_commands.json"

function require_compile_db() {
    if [ -f "${DB_FILE}" ]; then
        return 0
    fi

    if [ "${NO_CONFIGURE}" -eq 1 ]; then
        print_log "找不到 ${DB_FILE}，且指定了 --no-configure" error
        exit 1
    fi

    print_log "未找到编译数据库，先执行 configure: preset=${PRESET}" info
    if ! cmake --preset="${PRESET}"; then
        print_log "[${PRESET}] configure failed" error
        exit 1
    fi

    if [ ! -f "${DB_FILE}" ]; then
        print_log "configure 后仍未生成 ${DB_FILE}，请检查 CMAKE_EXPORT_COMPILE_COMMANDS" error
        exit 1
    fi
}

function refresh_top_db() {
    if [ "${DB_FILE}" = "${TOP_DB}" ]; then
        return 0
    fi
    if ! cp "${DB_FILE}" "${TOP_DB}"; then
        print_log "复制 ${DB_FILE} -> ${TOP_DB} 失败" error
        return 1
    fi
    return 0
}

# 校验 .clang-tidy 里声明的 check 是否都存在于当前 clang-tidy。
# 不存在的 check 会被 clang-tidy 静默忽略，等于该维度从未生效，这里显式拦下。
function verify_configured_checks() {
    local configured available unknown
    configured="$(
        awk '
            /^Checks:/ { inside = 1; next }
            inside && /^[^[:space:]]/ { inside = 0 }
            inside { print }
        ' .clang-tidy |
            tr ',' '\n' |
            sed 's/#.*$//' |
            tr -d ' \t' |
            grep -E '^[a-z][a-z0-9-]*$' |
            sort -u
    )"

    if [ -z "${configured}" ]; then
        print_log "未能从 .clang-tidy 解析出 Checks，跳过 check 名校验" debug
        return 0
    fi

    available="$("${CLANG_TIDY_BIN}" --list-checks --checks='*' 2>/dev/null | sed 's/^ *//' | sort -u)"
    unknown=""
    while IFS= read -r check; do
        if ! grep -qxF "${check}" <<<"${available}"; then
            unknown="${unknown} ${check}"
        fi
    done <<<"${configured}"

    if [ -n "${unknown}" ]; then
        print_log "以下 check 在当前 clang-tidy 中不存在，会被静默忽略:" error
        for check in ${unknown}; do
            print_log "  ${check}" error
        done
        return 1
    fi

    print_log "配置中的 check 全部有效（$(wc -l <<<"${configured}" | tr -d ' ') 个）" debug
    return 0
}

# 编译数据库里的路径是 Windows 形态（E:/...），Git Bash 下 pwd 是 /e/...，
# 用 cygpath -m 统一成混合形式再比对。
if command -v cygpath >/dev/null 2>&1; then
    ROOT_MIXED="$(cygpath -m "${ROOT_DIR}")"
else
    ROOT_MIXED="${ROOT_DIR}"
fi

function file_in_db() {
    local abs_path="$1"
    grep -qF "\"${abs_path}\"" "${DB_FILE}"
}

# GitHub workflow command 需要对 % 和换行做转义；属性值还要额外转义 , 和 :
function gh_escape() {
    local text="$1"
    text="${text//%/%25}"
    text="${text//$'\r'/%0D}"
    text="${text//$'\n'/%0A}"
    printf '%s' "${text}"
}

function gh_escape_prop() {
    local text
    text="$(gh_escape "$1")"
    text="${text//,/%2C}"
    text="${text//:/%3A}"
    printf '%s' "${text}"
}

# 把 clang-tidy 的 "path:line:col: warning: message" 转成 GitHub 注解，
# 这样告警会直接显示在 PR 的 Checks / Files changed 里，而不是只躺在日志中。
function emit_github_annotations() {
    local log_file="$1"
    local line path lineno col severity message relative
    while IFS= read -r line; do
        if [[ "${line}" =~ ^(.+):([0-9]+):([0-9]+):\ (warning|error):\ (.*)$ ]]; then
            path="${BASH_REMATCH[1]}"
            lineno="${BASH_REMATCH[2]}"
            col="${BASH_REMATCH[3]}"
            severity="${BASH_REMATCH[4]}"
            message="${BASH_REMATCH[5]}"
            path="${path//\\//}"
            relative="${path#${ROOT_MIXED}/}"
            relative="${relative#${ROOT_DIR}/}"
            printf '::%s file=%s,line=%s,col=%s::%s\n' \
                "${severity}" "$(gh_escape_prop "${relative}")" "${lineno}" "${col}" \
                "$(gh_escape "${message}")"
        fi
    done <"${log_file}"
}

function collect_files() {
    if [ "${ALL}" -eq 1 ]; then
        find src -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' \) | sort
    else
        {
            git diff --cached --name-only --diff-filter=ACMR
            git diff --name-only --diff-filter=ACMR
            git ls-files --others --exclude-standard
        } | sort -u | grep -E '^src/.*\.(c|cc|cpp|cxx)$' || true
    fi
}

require_compile_db
refresh_top_db || exit 1

if ! verify_configured_checks; then
    print_log "请删除或替换 .clang-tidy 中不存在的 check 后重试" error
    exit 1
fi

if [ "${PREPARE_DB}" -eq 1 ]; then
    print_log "编译数据库已就绪: ${TOP_DB}" info
    exit 0
fi

# macOS 自带 bash 3.2 没有 mapfile，这里用 read 循环保持兼容。
CANDIDATES=()
while IFS= read -r line; do
    if [ -n "${line}" ]; then
        CANDIDATES+=("${line}")
    fi
done < <(collect_files)

if [ "${#CANDIDATES[@]}" -eq 0 ]; then
    if [ "${ALL}" -eq 1 ]; then
        print_log "src 下没有可检查的源文件" info
    else
        print_log "没有改动文件；如需全量检查请加 --all" info
    fi
    exit 0
fi

FILES=()
SKIPPED=()
for file in "${CANDIDATES[@]}"; do
    if file_in_db "${ROOT_MIXED}/${file}"; then
        FILES+=("${file}")
    else
        SKIPPED+=("${file}")
    fi
done

if [ "${#SKIPPED[@]}" -gt 0 ]; then
    print_log "以下文件不在编译数据库里（新增文件请先重新 configure），已跳过:" debug
    for file in "${SKIPPED[@]}"; do
        print_log "  ${file}" debug
    done
fi

if [ "${#FILES[@]}" -eq 0 ]; then
    print_log "没有可检查的文件" error
    exit 1
fi

TIDY_ARGS=(-p "${DB_DIR}" --quiet)
if [ "${WERROR}" -eq 1 ]; then
    TIDY_ARGS+=(--warnings-as-errors='*')
fi
if [ "${APPLY_FIX}" -eq 1 ]; then
    TIDY_ARGS+=(--fix)
fi

print_log "clang-tidy 检查开始: preset=${PRESET} files=${#FILES[@]}" info

TIDY_STATUS=0
if [ "${GITHUB_ANNOTATIONS}" -eq 1 ]; then
    TIDY_LOG="$(mktemp)" || {
        print_log "创建临时文件失败" error
        exit 1
    }
    "${CLANG_TIDY_BIN}" "${TIDY_ARGS[@]}" "${FILES[@]}" >"${TIDY_LOG}" 2>&1 || TIDY_STATUS=$?
    cat "${TIDY_LOG}"
    emit_github_annotations "${TIDY_LOG}"

    DIAGNOSTIC_COUNT="$(grep -cE '^.+:[0-9]+:[0-9]+: (warning|error): ' "${TIDY_LOG}" || true)"
    print_log "诊断数: ${DIAGNOSTIC_COUNT:-0}" info
    if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
        {
            echo "### clang-tidy (${PRESET})"
            echo ""
            echo "- 文件数: ${#FILES[@]}"
            echo "- 诊断数: ${DIAGNOSTIC_COUNT:-0}"
        } >>"${GITHUB_STEP_SUMMARY}"
    fi
    rm -f "${TIDY_LOG}"
else
    "${CLANG_TIDY_BIN}" "${TIDY_ARGS[@]}" "${FILES[@]}" || TIDY_STATUS=$?
fi

if [ "${TIDY_STATUS}" -eq 0 ]; then
    print_log "clang-tidy 检查完成: ${#FILES[@]} 个文件" info
    exit 0
fi

print_log "clang-tidy 检查失败: ${#FILES[@]} 个文件" error
exit 1
