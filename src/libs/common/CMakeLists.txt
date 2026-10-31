set(LIB_NAME Common)

add_library(${LIB_NAME} SHARED)
target_compile_definitions(${LIB_NAME} PRIVATE COMMON_BUILD_SHARED)

set_lib_public_properties(${LIB_NAME})
