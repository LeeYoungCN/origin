# ---------------------------------------------------------------
# 为 Windows 二进制添加 VERSIONINFO 资源
#
# 用法（在 target 创建之后）：
#   origin_add_version_resource(<target> <输出名(不含后缀)> [<描述>])
#
# 数值字段取 CMake 的 ORIGIN_VERSION_MAJOR/MINOR/PATCH；
# FileVersion / ProductVersion 字符串使用完整的 ORIGIN_VERSION（含 -dev+g<sha>）。
# 非 Windows 平台直接返回，不影响其它平台构建。
# ---------------------------------------------------------------

set(ORIGIN_RC_COMPANY "LeeYoungCN" CACHE STRING "VERSIONINFO 的 CompanyName")
set(ORIGIN_RC_COPYRIGHT "Copyright (c) 2026 LeeYoungCN" CACHE STRING "VERSIONINFO 的 LegalCopyright")
set(ORIGIN_RC_PRODUCT_NAME "${PROJECT_NAME}" CACHE STRING "VERSIONINFO 的 ProductName")

function(origin_add_version_resource target output_name)
    if(NOT WIN32)
        return()
    endif()

    if(NOT DEFINED ORIGIN_RC_COMPANY OR "${ORIGIN_RC_COMPANY}" STREQUAL "")
        message(FATAL_ERROR "ORIGIN_RC_COMPANY is empty; include cmake/version_resource.cmake first")
    endif()

    set(description "${ARGV2}")
    if("${description}" STREQUAL "")
        set(description "${output_name} library of ${ORIGIN_RC_PRODUCT_NAME}")
    endif()

    # 配置相关的内容（FILEFLAGS、OriginalFilename 的 d 后缀）用 ORIGIN_RC_DEBUG 判断。
    # 这里显式带上 $<CONFIG:Debug> 的定义，不依赖各工具链是否给资源编译器传 -D_DEBUG：
    # 生成器表达式在 generate 阶段按配置求值，单配置/多配置生成器都成立。
    target_compile_definitions(${target} PRIVATE $<$<CONFIG:Debug>:ORIGIN_RC_DEBUG>)

    set(ORIGIN_RC_FILE_TYPE "DLL")
    set(ORIGIN_RC_INTERNAL_NAME "${output_name}")
    set(ORIGIN_RC_ORIGINAL_FILENAME "${output_name}")
    set(ORIGIN_RC_DESCRIPTION "${description}")
    set(ORIGIN_RC_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
    set(ORIGIN_RC_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
    set(ORIGIN_RC_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

    set(rc_file "${CMAKE_CURRENT_BINARY_DIR}/${target}.version.rc")
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/version.rc.in"
        "${rc_file}"
        @ONLY)

    target_sources(${target} PRIVATE "${rc_file}")
endfunction()
