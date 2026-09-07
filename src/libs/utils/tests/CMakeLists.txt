set_target_properties(${LIB_NAME} PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 自动导出所有符号
)

set(TEST_RUNTIME_NAME test_${LIB_NAME})

add_executable(${TEST_RUNTIME_NAME})

file(GLOB_RECURSE SRC_LIST "*.cpp")
target_sources(${TEST_RUNTIME_NAME} PRIVATE ${SRC_LIST})

target_include_directories(${TEST_RUNTIME_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>  # 编译时路径
        ${CMAKE_CURRENT_SOURCE_DIR}/../src                         # 内部头文件路径
)

target_link_libraries(${TEST_RUNTIME_NAME} PUBLIC ${LIB_NAME} GTest::gtest GTest::gtest_main)

if (MINGW)
    target_link_libraries(${TEST_RUNTIME_NAME} PUBLIC pthread)
endif()

include(GoogleTest)
gtest_discover_tests(${TEST_RUNTIME_NAME})
