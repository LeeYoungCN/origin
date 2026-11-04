set(LIB_NAME Common)

file(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
add_library(${LIB_NAME} SHARED ${SRC_LIST})
target_compile_definitions(${LIB_NAME} PRIVATE COMMON_BUILD_SHARED)

origin_add_component(${LIB_NAME})
