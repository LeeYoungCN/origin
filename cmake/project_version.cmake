# ---------------------------------------------------------------
# 版本号解析（在 project() 之前 include）
#
# 调用方约定：
#   set(ORIGIN_VERSION_FILE "1.0.1")          # 树内唯一来源，发版时改这一行
#   include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/project_version.cmake)
#   project(Origin VERSION ${ORIGIN_VERSION_NUMERIC} LANGUAGES C CXX)
#
# 可选 -D 覆盖：
#   -DORIGIN_VERSION=x.y.z   只影响当前构建目录，会留在缓存里（用 -UORIGIN_VERSION 清除）
#   -DORIGIN_DEV_SUFFIX=OFF  关闭开发版后缀（发布构建用）
#
# 输出变量：
#   ORIGIN_VERSION          语义化版本，形如 1.0.1（进 version.h，不随提交变化）
#   ORIGIN_VERSION_FULL     开发版完整串，形如 1.0.1-dev+g7b4ef62-dirty（只进 build_info.h）
#   ORIGIN_VERSION_NUMERIC  纯数字部分，供 project(VERSION) / SOVERSION 使用
#   ORIGIN_VERSION_COMMIT   短 sha，发布构建为空
#   ORIGIN_VERSION_DIRTY    工作区是否脏（0/1）
# ---------------------------------------------------------------

if(NOT DEFINED ORIGIN_VERSION_FILE OR "${ORIGIN_VERSION_FILE}" STREQUAL "")
    message(FATAL_ERROR "ORIGIN_VERSION_FILE is not set; set it before including project_version.cmake")
endif()

option(ORIGIN_DEV_SUFFIX "非发布构建在版本号后追加 -dev+g<sha>" ON)

# 供 build_info.h 使用：发布构建时提交号为空、dirty 为 0
set(ORIGIN_VERSION_COMMIT "")
set(ORIGIN_VERSION_DIRTY 0)

# 1) 基础版本：-D 覆盖优先，否则以树内声明为准
if(DEFINED ORIGIN_VERSION AND NOT "${ORIGIN_VERSION}" STREQUAL "")
    if(NOT "${ORIGIN_VERSION}" STREQUAL "${ORIGIN_VERSION_FILE}")
        message(
            WARNING
                "build cache has ORIGIN_VERSION=${ORIGIN_VERSION}, CMakeLists has ${ORIGIN_VERSION_FILE}; "
                "reconfigure with -UORIGIN_VERSION to follow the file")
    endif()
else()
    set(ORIGIN_VERSION "${ORIGIN_VERSION_FILE}")
endif()

# 2) 非发布构建追加 git 状态：HEAD 正好停在 v<基础版本> 上才算发布构建
#    追加结果只进 ORIGIN_VERSION_FULL，ORIGIN_VERSION 保持语义化版本不变，
#    这样每次提交都不会改动 version.h，也就不会触发全量重编。
set(ORIGIN_VERSION_FULL "${ORIGIN_VERSION}")
if(ORIGIN_DEV_SUFFIX)
    execute_process(
        COMMAND git describe --tags --exact-match
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _origin_exact_tag
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)

    if(NOT _origin_exact_tag STREQUAL "v${ORIGIN_VERSION}")
        execute_process(
            COMMAND git rev-parse --short HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE _origin_head_sha
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)

        if(_origin_head_sha)
            execute_process(
                COMMAND git status --porcelain --untracked-files=no
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE _origin_dirty
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)

            set(ORIGIN_VERSION_FULL "${ORIGIN_VERSION_FULL}-dev+g${_origin_head_sha}")
            set(ORIGIN_VERSION_COMMIT "${_origin_head_sha}")
            if(_origin_dirty)
                set(ORIGIN_VERSION_FULL "${ORIGIN_VERSION_FULL}-dirty")
                set(ORIGIN_VERSION_DIRTY 1)
            endif()
            message(STATUS "Origin build version: ${ORIGIN_VERSION_FULL}")
        endif()
    endif()
endif()

# 3) project(VERSION) 只接受纯数字，开发/预发布后缀在这里剥掉
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" ORIGIN_VERSION_NUMERIC "${ORIGIN_VERSION}")
if(NOT ORIGIN_VERSION_NUMERIC)
    message(WARNING "ORIGIN_VERSION '${ORIGIN_VERSION}' has no numeric prefix, fallback to 0.0.0")
    set(ORIGIN_VERSION_NUMERIC "0.0.0")
endif()
