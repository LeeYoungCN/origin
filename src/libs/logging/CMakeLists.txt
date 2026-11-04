set(LIB_NAME Logging)

file(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
add_library(${LIB_NAME} SHARED ${SRC_LIST})
target_compile_definitions(${LIB_NAME} PRIVATE LOGGING_BUILD_SHARED)

origin_add_component(${LIB_NAME})

target_link_libraries(${LIB_NAME} PUBLIC Common Container Utils)


if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
