#!/usr/local/bin/bash
echo "Bash version: $BASH_VERSION"
if [ -z "$BASHRC_LOADED" ]; then
    # shellcheck disable=SC1091
    source "${HOME}/.bashrc"
fi

SCRIPT_DIR="$(
    cd "$(dirname "$0")" || exit 1
    pwd
)"
ROOT_DIR="$(
    cd "${SCRIPT_DIR}/.." || exit 1
    pwd
)"

readonly SCRIPT_DIR
readonly ROOT_DIR

OS=$(uname -s)
if echo "${OS}" | grep -q "MINGW"; then
    OS="Windows"
fi

readonly OS

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/common_func.sh"

cd "${ROOT_DIR}" || exit 1

readonly BUILD_CACHE_ROOT_DIR="${ROOT_DIR}/out/build"
readonly TOOLCHAIN_FILE_DIR="${ROOT_DIR}/cmake/toolchain_files"
readonly INSTALL_ROOT_DIR="${ROOT_DIR}/out/install"

arg_preset=""

preset_array=("")

cmake_source_dir="${ROOT_DIR}"
cmake_binary_dir=""
cmake_test_runtime=""
cmake_install_prefix=""
cmake_build_type=""
cmake_toolchain_file=""
cmake_generator=""
cmake_preset=""
cmake_configure_param_cfg=""
# shellcheck disable=SC2034
ARCHITECTURE="x64"
env_param_file=""
cmake_enable_asan="OFF"
cmake_enable_test="OFF"

function init_cmake_preset() {
    case ${OS} in
    Windows)
        cmake_preset="msvc_x64_debug"
        preset_array=("msvc_x64_debug" "msvc_x64_release" "msvc_x86_debug" "msvc_x86_release" "win_mingw_debug" "win_mingw_release" "win_clang_msvc_debug" "win_clang_msvc_release")
        ;;
    Linux)
        cmake_preset="linux_clang_debug"
        preset_array=("linux_gnu_debug" "linux_gnu_release" "linux_clang_debug" "linux_clang_release")
        ;;
    Darwin)
        cmake_preset="darwin_clang_debug"
        preset_array=("darwin_clang_debug" "darwin_clang_release")
        ;;
    *)
        print_log "OS: ${OS} error!"
        exit 1
        ;;
    esac

    if [ -n "${arg_preset}" ] && [ "${arg_preset}" != "all" ]; then
        cmake_preset="${arg_preset}"
    fi
    cmake_binary_dir="${BUILD_CACHE_ROOT_DIR}/${cmake_preset}"
}

function init_cmake_configure_param() {
    case "${OS}" in
    Windows)
        case "${cmake_preset}" in
        win_mingw_debug)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_mingw.cmake"
            cmake_generator="MinGW Makefiles"
            cmake_build_type="Debug"
            ;;
        win_mingw_release)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_mingw.cmake"
            cmake_generator="MinGW Makefiles"
            cmake_build_type="Release"
            ;;
        win_clang_msvc_debug)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_clang_msvc.cmake"
            cmake_generator="Ninja"
            cmake_build_type="Debug"
            ;;
        win_clang_msvc_release)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_clang_msvc.cmake"
            cmake_generator="Ninja"
            cmake_build_type="Release"
            ;;
        win_clang_mingw_debug)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_clang_mingw.cmake"
            cmake_generator="MinGW Makefiles"
            cmake_build_type="Debug"
            ;;
        win_clang_mingw_release)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_clang_mingw.cmake"
            cmake_generator="MinGW Makefiles"
            cmake_build_type="Release"
            ;;
        msvc_x64_debug)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_msvc.cmake"
            cmake_generator="Ninja"
            cmake_build_type="Debug"
            env_param_file="${SCRIPT_DIR}/load_msvc_env.sh"
            ;;
        msvc_x64_release)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_msvc.cmake"
            cmake_generator="Ninja"
            cmake_build_type="Release"
            env_param_file="${SCRIPT_DIR}/load_msvc_env.sh"
            ;;
        msvc_x86_debug)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_msvc.cmake"
            cmake_generator="Ninja"
            cmake_build_type="Debug"
            ARCHITECTURE="x86"
            env_param_file="${SCRIPT_DIR}/load_msvc_env.sh"
            ;;
        msvc_x86_release)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_msvc.cmake"
            cmake_generator="Ninja"
            cmake_build_type="Release"
            # shellcheck disable=SC2034
            ARCHITECTURE="x86"
            env_param_file="${SCRIPT_DIR}/load_msvc_env.sh"
            ;;
        win_clang_cl_debug)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_clang_cl.cmake"
            cmake_generator="Ninja"
            cmake_build_type="Debug"
            ;;
        win_clang_cl_release)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/win_clang_cl.cmake"
            cmake_generator="Ninja"
            cmake_build_type="Release"
            ;;
        *)
            print_log "Preset: ${arg_preset} error!" error
            exit 1
            ;;
        esac
        ;;
    Linux)
        case "${cmake_preset}" in
        linux_gnu_debug)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/linux_gnu.cmake"
            cmake_generator="Unix Makefiles"
            cmake_build_type="Debug"
            ;;
        linux_gnu_release)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/linux_gnu.cmake"
            cmake_generator="Unix Makefiles"
            cmake_build_type="Release"
            ;;
        linux_clang_debug)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/linux_clang.cmake"
            cmake_generator="Unix Makefiles"
            cmake_build_type="Debug"
            ;;
        linux_clang_release)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/linux_clang.cmake"
            cmake_generator="Unix Makefiles"
            cmake_build_type="Release"
            ;;
        *)
            print_log "Preset: ${arg_preset} error!" error
            exit 1
            ;;
        esac
        ;;
    Darwin)
        case "${cmake_preset}" in
        darwin_clang_debug)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/darwin_clang.cmake"
            cmake_generator="Unix Makefiles"
            cmake_build_type="Debug"
            ;;
        darwin_clang_release)
            cmake_toolchain_file="${TOOLCHAIN_FILE_DIR}/darwin_clang.cmake"
            cmake_generator="Unix Makefiles"
            cmake_build_type="Release"
            ;;
        *)
            print_log "Preset: ${arg_preset} error!" error
            exit 1
            ;;
        esac
        ;;
    *)
        print_log "OS: ${OS} error!"
        exit 1
        ;;
    esac

    if [ -z "${cmake_install_prefix}" ]; then
        cmake_install_prefix="${INSTALL_ROOT_DIR}/${cmake_preset}"
    fi

    if [ -n "${env_param_file}" ]; then
        # shellcheck disable=SC1090
        source "${env_param_file}"
    fi

    print_log "Init CMake configure param success." info
}

function cmake_configure() {
    rm_dir "${cmake_binary_dir}"
    init_cmake_configure_param

    if cmake \
        -S "${cmake_source_dir}" \
        -B "${cmake_binary_dir}" \
        -G "${cmake_generator}" \
        -DCMAKE_TOOLCHAIN_FILE="${cmake_toolchain_file}" \
        -DCMAKE_BUILD_TYPE="${cmake_build_type}" \
        -DCMAKE_INSTALL_PREFIX="${cmake_install_prefix}" \
        -DCMAKE_PRESET="${cmake_preset}" \
        -DENV_PARAM_FILE="${env_param_file}" \
        -DENABLE_ASAN="${cmake_enable_asan}" \
        -DBUILD_TESTS="${cmake_enable_test}"; then
        print_log "[${cmake_preset}] CMake configuration success." info
    else
        print_log "[${cmake_preset}] CMake configuration failed." error
        exit 1
    fi
}

function cmake_build() {
    if cmake --build "${cmake_binary_dir}" -j4; then
        print_log "[${cmake_preset}] CMake build success." info
    else
        print_log "[${cmake_preset}] CMake build failed." error
        exit 1
    fi

}

function cmake_install() {
    rm_dir "${cmake_install_prefix}"
    if cmake --install "${cmake_binary_dir}"; then
        print_log "[${cmake_preset}] CMake install all success." info
    else
        print_log "[${cmake_preset}] CMake install all failed." error
        exit 1
    fi
}

function main() {
    if ! ARGS=$(
        getopt -o p: --long install:,preset: \
            -n "$0" -- "$@"
    ); then
        print_log "getopt failed." error
        exit 1
    fi

    eval set -- "$ARGS"

    while true; do
        case "$1" in
        --preset)
            arg_preset="${2}"
            shift 2
            ;;
        --install)
            cmake_install_prefix="${2}"
            shift 2
            ;;
        --)
            shift 1
            break
            ;;
        *)
            print_log "Invalid getopt param [${1}]." error
            exit 1
            ;;
        esac
    done

    init_cmake_preset
    cmake_configure
    cmake_build
    cmake_install
}

main "$@"
