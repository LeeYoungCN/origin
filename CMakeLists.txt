cmake_minimum_required(VERSION 3.14)

# 版本号唯一来源：发版时改这里，并打同名 tag（v<版本号>），CI 负责校验两者一致。
# 解析逻辑（-D 覆盖、开发版后缀、纯数字提取）见 cmake/version/project_version.cmake。
set(ORIGIN_VERSION_FILE "2.0.0")
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version/project_version.cmake)

project(Origin VERSION ${ORIGIN_VERSION_NUMERIC} LANGUAGES C CXX)

# FetchContent / add_subdirectory 嵌入时，CMAKE_SOURCE_DIR 指向父项目，
# 这里区分是否作为顶层项目构建，用于决定要不要改全局设置。
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(ORIGIN_TOP_LEVEL TRUE)
else()
    set(ORIGIN_TOP_LEVEL FALSE)
endif()

# Windows 下为 DLL 嵌入 VERSIONINFO 资源（资源管理器/GetFileVersionInfo 可读版本）
if(WIN32)
    enable_language(RC)
endif()

# 版本头：产物里能查到编译时使用的版本
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version/version.h.in"
    "${CMAKE_BINARY_DIR}/generated/origin/version.h"
    @ONLY)

# 开发版信息（完整串 / 提交号 / 是否脏）单独生成，只有 Common 的 version.cpp 引用，
# 避免每次提交或 reconfigure 触发全量重编。
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version/build_info.h.in"
    "${CMAKE_BINARY_DIR}/generated/origin/build_info.h"
    @ONLY)

set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)
set(CMAKE_VERBOSE_MAKEFILE          OFF)

# 输出目录只在顶层工程设置；被 FetchContent/add_subdirectory 嵌入时不要覆盖父工程
if (ORIGIN_TOP_LEVEL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

option(ORIGIN_BUILD_TESTS "Build and run tests." OFF)
option(ORIGIN_ENABLE_ASAN "Enable asan."         OFF)

if (MSVC)
    # CMake 默认 flags 带 /W3，目标级再加 /W4 会触发 D9025（overriding /W3 with /W4）；
    # 这里先摘掉 /W3，警告级别统一由 ORIGIN_WARNING_OPTIONS 控制
    string(REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    # 摘掉 /W3 后会留下多余空格，顺手压一下
    string(REGEX REPLACE " +" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE " +" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

endif()

if (MSVC)
    # MSVC 与 clang-cl 都用 /W4；clang-cl 的 -Wall 等价于 /Wall，
    # 会把 -Wc++98-compat、-Wpadded 这类噪音全部打开
    set(ORIGIN_WARNING_OPTIONS "/W4")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(ORIGIN_WARNING_OPTIONS "-Wall" "-Wextra" "-Wconversion" "-pedantic")
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Default CMAKE_BUILD_TYPE Debug" FORCE)
endif()



include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/preset_info.cmake)

# 被 FetchContent / add_subdirectory 嵌入时，不要覆盖父项目的全局设置
if(ORIGIN_TOP_LEVEL)
    set(_origin_default_prefixes
        "C:/Program Files (x86)/${PROJECT_NAME}"
        "C:/Program Files/${PROJECT_NAME}"
        "/usr/local")
    if (NOT CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX IN_LIST _origin_default_prefixes)
        set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/out/install/${CMAKE_PRESET}" CACHE PATH "Default CMAKE_INSTALL_PREFIX out/install/${CMAKE_PRESET}" FORCE)
    endif()

    if (NOT CMAKE_THIRD_PARTY_PATH)
        set(CMAKE_THIRD_PARTY_PATH "${PROJECT_SOURCE_DIR}/third_party/${CMAKE_PRESET}" CACHE STRING "Default CMAKE_THIRD_PARTY_PATH third_party" FORCE)
    endif()

    if (NOT CPACK_PACKAGE_DIRECTORY)
        set(CPACK_PACKAGE_DIRECTORY "${PROJECT_SOURCE_DIR}/out/pkg/${CMAKE_PRESET}")
    endif()
endif()

if (ORIGIN_ENABLE_ASAN)
    if (MSVC)
        # /RTC 与 ASan 不兼容，C 和 C++ 的 debug flags 都要清理
        foreach(flag CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG)
            string(REGEX REPLACE "/RTC(su|[1su])" "" ${flag} "${${flag}}")
        endforeach()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    else()
        message(WARNING "ASan is not supported on this platform/compiler.")
    endif()
endif()

if (ORIGIN_TOP_LEVEL AND NOT EXISTS "${CMAKE_THIRD_PARTY_PATH}")
    message(WARNING "CMAKE_THIRD_PARTY_PATH does not exist: ${CMAKE_THIRD_PARTY_PATH}")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure/configure_summary.cmake)

if (ORIGIN_BUILD_TESTS)
    enable_testing()
endif()

include(GNUInstallDirs)

# 安装产物的 RPATH：按相对位置查找同一 install 树里的库，整个目录可搬迁
if (APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif (UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

add_subdirectory(src)

# ---------------------------------------------------------------
# CPack：标准打包入口（cmake_tools.sh --pkg 会优先用它）
#
#   cpack --config out/build/<preset>/CPackConfig.cmake -G ZIP -B out/pkg
#
# 只打安装树（include/lib/bin + lib/cmake/Origin），解压后可直接被
# CMAKE_PREFIX_PATH + find_package(Origin CONFIG) 使用。
# CPACK_PACKAGE_VERSION 必须是纯数字（dev 后缀会破坏版本比较），
# 完整版本（含 -dev+g<sha>）放在包名里。
# ---------------------------------------------------------------
set(CPACK_PACKAGE_NAME "origin")
set(CPACK_PACKAGE_VENDOR "LeeYoungCN")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Origin C++ libraries")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "origin-${ORIGIN_VERSION_FULL}-${CMAKE_PRESET}")
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
if(ORIGIN_TOP_LEVEL)
    include(CPack)
endif()
