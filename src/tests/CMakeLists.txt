set(TEST_RUNTIME_NAME origin_test)
add_executable(${TEST_RUNTIME_NAME})

file(GLOB_RECURSE SRC_LIST "*.cpp")

target_sources(${TEST_RUNTIME_NAME} PRIVATE ${SRC_LIST})

target_link_libraries(${TEST_RUNTIME_NAME} GTest::gtest ${LIB_LIST} container_test)

# 添加ctest测试
add_test(NAME TEST_ALL COMMAND ${TEST_RUNTIME_NAME})
set_tests_properties(TEST_ALL PROPERTIES TIMEOUT 60)
