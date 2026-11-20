include(ProcessorCount)
ProcessorCount(CPU_CORES)

if(CPU_CORES EQUAL 0)
    set(CPU_CORES 1)
endif()

set(CMAKE_CONFIGURE_FILE  ${CMAKE_BINARY_DIR}/cmake_configure.conf)
set(CMAKE_CONFIGURE_SRC   ${CMAKE_CURRENT_LIST_DIR}/configure_summary.conf.in)

configure_file(
    "${CMAKE_CONFIGURE_SRC}"
    "${CMAKE_CONFIGURE_FILE}"
)

function(origin_summary_print key value)
    string(LENGTH "${key}" _key_len)
    math(EXPR _padding "40 - ${_key_len}")
    if (_padding LESS 1)
        set(_padding 1)
    endif()
    string(REPEAT " " ${_padding} _spaces)
    message(STATUS "${key}:${_spaces}${value}")
endfunction()

# 构建配置
origin_summary_print(CMAKE_PRESET          "${CMAKE_PRESET}")
origin_summary_print(CMAKE_BUILD_TYPE      "${CMAKE_BUILD_TYPE}")
origin_summary_print(CMAKE_GENERATOR       "${CMAKE_GENERATOR}")
origin_summary_print(CMAKE_TOOLCHAIN_FILE  "${CMAKE_TOOLCHAIN_FILE}")
origin_summary_print(ENV_PARAM_FILE        "${ENV_PARAM_FILE}")
origin_summary_print(CMAKE_CONFIGURE_FILE  "${CMAKE_CONFIGURE_FILE}")

# 版本
origin_summary_print(ORIGIN_VERSION        "${ORIGIN_VERSION}")
origin_summary_print(ORIGIN_VERSION_FULL   "${ORIGIN_VERSION_FULL}")
origin_summary_print(ORIGIN_VERSION_NUMERIC "${ORIGIN_VERSION_NUMERIC}")

# 路径
origin_summary_print(PROJECT_SOURCE_DIR    "${PROJECT_SOURCE_DIR}")
origin_summary_print(CMAKE_SOURCE_DIR      "${CMAKE_SOURCE_DIR}")
origin_summary_print(CMAKE_BINARY_DIR      "${CMAKE_BINARY_DIR}")
origin_summary_print(CMAKE_INSTALL_PREFIX  "${CMAKE_INSTALL_PREFIX}")
origin_summary_print(CPACK_PACKAGE_DIRECTORY "${CPACK_PACKAGE_DIRECTORY}")
origin_summary_print(CMAKE_THIRD_PARTY_PATH "${CMAKE_THIRD_PARTY_PATH}")

# 功能开关
origin_summary_print(ORIGIN_BUILD_TESTS    "${ORIGIN_BUILD_TESTS}")
origin_summary_print(ORIGIN_ENABLE_ASAN    "${ORIGIN_ENABLE_ASAN}")
origin_summary_print(ORIGIN_INSTALL        "${ORIGIN_INSTALL}")

# 平台
origin_summary_print(CMAKE_SYSTEM_NAME     "${CMAKE_SYSTEM_NAME}")
origin_summary_print(CMAKE_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
origin_summary_print(CMAKE_SIZEOF_VOID_P   "${CMAKE_SIZEOF_VOID_P}")
origin_summary_print(CPU_CORES             "${CPU_CORES}")
origin_summary_print(CMAKE_VERSION         "${CMAKE_VERSION}")

# 编译器
origin_summary_print(CMAKE_C_COMPILER      "${CMAKE_C_COMPILER}")
origin_summary_print(CMAKE_C_COMPILER_ID   "${CMAKE_C_COMPILER_ID}")
origin_summary_print(CMAKE_C_COMPILER_VERSION "${CMAKE_C_COMPILER_VERSION}")
origin_summary_print(CMAKE_C_COMPILER_FRONTEND_VARIANT "${CMAKE_C_COMPILER_FRONTEND_VARIANT}")
origin_summary_print(CMAKE_CXX_COMPILER    "${CMAKE_CXX_COMPILER}")
origin_summary_print(CMAKE_CXX_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
origin_summary_print(CMAKE_CXX_COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
origin_summary_print(CMAKE_CXX_COMPILER_FRONTEND_VARIANT "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")

# 编译/链接选项
origin_summary_print(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}")
origin_summary_print(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}")
origin_summary_print(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
origin_summary_print(CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS}")
origin_summary_print(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
origin_summary_print(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
origin_summary_print(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
