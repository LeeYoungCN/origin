cmake_minimum_required(VERSION 3.14)

# 版本号唯一来源：发版时改这里，并打同名 tag（v<ORIGIN_VERSION>），CI 负责校验两者一致。
# 以普通变量（而非 CACHE）给出，改这一行会对已有构建目录立即生效；
# -DORIGIN_VERSION=x.y.z 可临时覆盖，但会留在缓存里，需用 -UORIGIN_VERSION 清除。
set(ORIGIN_VERSION_FILE "1.0.1")
if(DEFINED ORIGIN_VERSION)
    if(NOT ORIGIN_VERSION STREQUAL ORIGIN_VERSION_FILE)
        message(
            WARNING
                "build cache has ORIGIN_VERSION=${ORIGIN_VERSION}, CMakeLists has ${ORIGIN_VERSION_FILE}; "
                "reconfigure with -UORIGIN_VERSION to follow the file")
    endif()
else()
    set(ORIGIN_VERSION "${ORIGIN_VERSION_FILE}")
endif()

option(ORIGIN_DEV_SUFFIX "非发布构建在版本号后追加 -dev+g<sha>" ON)

# 非发布构建（本地 / 分支）自动带上 git 状态，避免自称已发布版本。
# 判据：HEAD 正好停在 v<ORIGIN_VERSION> 上才算发布构建。
if(ORIGIN_DEV_SUFFIX)
    execute_process(
        COMMAND git describe --tags --exact-match
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE ORIGIN_EXACT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    if(NOT ORIGIN_EXACT_TAG STREQUAL "v${ORIGIN_VERSION}")
        execute_process(
            COMMAND git rev-parse --short HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE ORIGIN_HEAD_SHA
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)
        if(ORIGIN_HEAD_SHA)
            execute_process(
                COMMAND git status --porcelain --untracked-files=no
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE ORIGIN_DIRTY
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
            set(ORIGIN_VERSION "${ORIGIN_VERSION}-dev+g${ORIGIN_HEAD_SHA}")
            if(ORIGIN_DIRTY)
                set(ORIGIN_VERSION "${ORIGIN_VERSION}-dirty")
            endif()
            message(STATUS "Origin dev version: ${ORIGIN_VERSION}")
        endif()
    endif()
endif()

# project(VERSION) 只接受纯数字，开发/预发布后缀要剥掉；
# 完整字符串仍通过 ORIGIN_VERSION_STRING 暴露给库内版本查询。
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" ORIGIN_VERSION_NUMERIC "${ORIGIN_VERSION}")
if(NOT ORIGIN_VERSION_NUMERIC)
    message(WARNING "ORIGIN_VERSION '${ORIGIN_VERSION}' has no numeric prefix, fallback to 0.0.0")
    set(ORIGIN_VERSION_NUMERIC "0.0.0")
endif()

project(Origin VERSION ${ORIGIN_VERSION_NUMERIC} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)
set(CMAKE_VERBOSE_MAKEFILE          OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(BUILD_TESTS "Build and run tests."   OFF)
option(ENABLE_ASAN "Enable asan."           OFF)

if (MSVC)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    add_compile_options(/permissive- /utf-8 /Zc:__cplusplus)
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(/Zc:preprocessor)
    endif()
endif()

if (MINGW OR CMAKE_COMPILER_IS_GNUCXX OR
    (CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang" AND
     CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU"))
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -O0")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    add_compile_options("-Wall" "-Wconversion" "-Wextra" "-pedantic")
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Default CMAKE_BUILD_TYPE Debug" FORCE)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_MODE)
else()
    add_compile_definitions(NDEBUG_MODE)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/get_cmake_preset.cmake)

if (NOT CMAKE_INSTALL_PREFIX OR
    CMAKE_INSTALL_PREFIX STREQUAL "C:/Program Files (x86)/${PROJECT_NAME}" OR
    CMAKE_INSTALL_PREFIX STREQUAL "C:/Program Files/${PROJECT_NAME}" OR
    CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/out/install/${CMAKE_PRESET}" CACHE PATH "Default CMAKE_INSTALL_PREFIX out/install/${CMAKE_PRESET}" FORCE)
endif()

if (NOT CMAKE_THIRD_PARTY_PATH)
    set(CMAKE_THIRD_PARTY_PATH "${CMAKE_SOURCE_DIR}/third_party/${CMAKE_PRESET}" CACHE STRING "Default CMAKE_THIRD_PARTY_PATH third_party" FORCE)
endif()

if (ENABLE_ASAN)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(MSVC)
            add_compile_options(/fsanitize=address /Zi)
            add_link_options(/fsanitize=address)
            # 关闭不兼容项
            string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
            string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
            set(CMAKE_THIRD_PARTY_PATH "")
        else()
            message(WARNING "ASan is not supported on this platform/compiler.")
        endif()
    else()
        if (CMAKE_COMPILER_IS_GNUCXX OR
            (CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang"))
            add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
            add_link_options(-fsanitize=address)
        else()
            message(WARNING "ASan is not supported on this platform/compiler.")
        endif()
    endif()
endif()

if (NOT EXISTS "${CMAKE_THIRD_PARTY_PATH}")
    message(WARNING "CMAKE_THIRD_PARTY_PATH does not exist: ${CMAKE_THIRD_PARTY_PATH}")
endif()

include(${CMAKE_SOURCE_DIR}/cmake/cmake_configure_info.cmake)

if (BUILD_TESTS)
    enable_testing()
    add_compile_definitions(ENABLE_TEST)
endif()

include(GNUInstallDirs)

# 安装产物的 RPATH：按相对位置查找同一 install 树里的库，整个目录可搬迁
if (APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif (UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

# 构建时间：取自 configure 阶段的本地时间，带时区偏移（%z），避免跨时区歧义
string(TIMESTAMP ORIGIN_BUILD_TIMESTAMP "%Y-%m-%dT%H:%M:%S%z")

# 版本头：产物里能查到编译时使用的版本
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/version.h.in"
    "${CMAKE_BINARY_DIR}/generated/origin/version.h"
    @ONLY)

add_subdirectory(src)
