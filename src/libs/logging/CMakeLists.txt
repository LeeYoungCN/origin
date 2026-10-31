set(LIB_NAME Logging)

add_library(${LIB_NAME} SHARED)
target_compile_definitions(${LIB_NAME} PRIVATE LOGGING_BUILD_SHARED)

set_lib_public_properties(${LIB_NAME})

target_link_libraries(${LIB_NAME} PUBLIC Common Container Utils)


if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
