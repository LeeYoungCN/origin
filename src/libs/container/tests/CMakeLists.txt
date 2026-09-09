set(TEST_RUNTIME_NAME test_${LIB_NAME})

add_executable(${TEST_RUNTIME_NAME})

file(GLOB_RECURSE SRC_LIST "*.cpp")
target_sources(${TEST_RUNTIME_NAME} PRIVATE ${SRC_LIST})

target_link_libraries(${TEST_RUNTIME_NAME} PUBLIC ${LIB_NAME} common utils GTest::gtest)

if (MINGW)
    target_link_libraries(${TEST_RUNTIME_NAME} PUBLIC pthread)
endif()

include(GoogleTest)
gtest_discover_tests(${TEST_RUNTIME_NAME})
