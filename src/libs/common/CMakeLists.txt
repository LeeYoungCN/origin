set(LIB_NAME Common)

# 显式源文件清单：新增/删除源文件时同步改这里。
# 不用 file(GLOB)，避免新增文件不触发重配导致漏编，也避免把临时/备份文件带进目标。
set(SRC_LIST
    src/c/version_c.cpp
    src/debug/debug_logger.cpp
    src/debug/working_env.cpp
    src/debug/working_env_c.c
    src/version.cpp
)
add_library(${LIB_NAME} SHARED ${SRC_LIST})
target_compile_definitions(${LIB_NAME} PRIVATE COMMON_BUILD_SHARED)

origin_add_component(${LIB_NAME})
