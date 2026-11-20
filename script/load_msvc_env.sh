#!/bin/bash
# 为 Git Bash / MSYS2 导入 MSVC 命令行编译所需的最小环境。
# 只覆盖 Ninja + cl.exe 的构建场景；需要 MSBuild / vcpkg / C++/WinRT 时请改用
# vcvarsall.bat（或脚本 load_msvc_env.ps1），不要往这里继续堆 VS 的完整环境。
if [ -z "${ARCHITECTURE}" ]; then
    ARCHITECTURE="x64"
fi

if [ "${ARCHITECTURE}" != "x64" ] && [ "${ARCHITECTURE}" != "x86" ]; then
    ARCHITECTURE="x64"
fi

VS_PATH="/D/ProgramFiles/Microsoft Visual Studio/18/Community"
MSVC_VERSION="14.52.37025"
WIN_KITS_10="/C/Program Files (x86)/Windows Kits/10"
WINSDK_VERSION="10.0.26100.0"

# 把一组目录按原顺序放到列表最前面，并先移除列表中已有的同名项（重复 source 不叠加）。
prepend_unique() {
    local current="$1"
    shift
    local prefix=""
    local item
    for item in "$@"; do
        current=":${current}:"
        current="${current//:${item}:/:}"
        current="${current#:}"
        current="${current%:}"
        if [ -z "${prefix}" ]; then
            prefix="${item}"
        else
            prefix="${prefix}:${item}"
        fi
    done
    if [ -n "${current}" ]; then
        printf '%s:%s' "${prefix}" "${current}"
    else
        printf '%s' "${prefix}"
    fi
}

########## PATH ##########
# MSVC 编译器 + Windows SDK 的 rc.exe / mt.exe
PATH_ARRAY=(
    "${VS_PATH}/VC/Tools/MSVC/${MSVC_VERSION}/bin/Host${ARCHITECTURE}/${ARCHITECTURE}"
    "${WIN_KITS_10}/bin/${WINSDK_VERSION}/${ARCHITECTURE}"
)
PATH="$(prepend_unique "${PATH}" "${PATH_ARRAY[@]}")"
export PATH

########## INCLUDE ##########
# MSVC 标准库 + SDK 的 ucrt/um/shared（winrt/cppwinrt 仅 C++/WinRT 需要）
INCLUDE_ARRAY=(
    "${VS_PATH}/VC/Tools/MSVC/${MSVC_VERSION}/include"
    "${WIN_KITS_10}/include/${WINSDK_VERSION}/ucrt"
    "${WIN_KITS_10}/include/${WINSDK_VERSION}/um"
    "${WIN_KITS_10}/include/${WINSDK_VERSION}/shared"
)
INCLUDE="$(prepend_unique "${INCLUDE}" "${INCLUDE_ARRAY[@]}")"
export INCLUDE

########## LIB ##########
# 原生链接只需要 MSVC 库 + SDK ucrt/um；LIBPATH 是 MSBuild/WinRT 用的，这里不设。
LIB_ARRAY=(
    "${VS_PATH}/VC/Tools/MSVC/${MSVC_VERSION}/lib/${ARCHITECTURE}"
    "${WIN_KITS_10}/lib/${WINSDK_VERSION}/ucrt/${ARCHITECTURE}"
    "${WIN_KITS_10}/lib/${WINSDK_VERSION}/um/${ARCHITECTURE}"
)
LIB="$(prepend_unique "${LIB}" "${LIB_ARRAY[@]}")"
export LIB
