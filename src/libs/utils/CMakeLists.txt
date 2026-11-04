set(LIB_NAME Utils)

file(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
add_library(${LIB_NAME} SHARED ${SRC_LIST})
target_compile_definitions(${LIB_NAME} PRIVATE UTILS_BUILD_SHARED)

origin_add_component(${LIB_NAME})

set_target_properties(${LIB_NAME} PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 自动导出所有符号
)

target_link_libraries(${LIB_NAME} PUBLIC Common)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
