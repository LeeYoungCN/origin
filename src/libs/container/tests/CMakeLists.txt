set(TEST_RUNTIME test_${LIB_NAME})
set(TEST_OBJECT  test_${LIB_NAME}_obj)

# 显式源文件清单：新增/删除测试文件时同步改这里。
set(SRC_LIST
    src/test_blocking_queue.cpp
    src/test_concurrent_blocking_queue_mt.cpp
    src/test_concurrent_blocking_queue_st.cpp
    src/test_cstring.cpp
)

add_library(${TEST_OBJECT} OBJECT ${SRC_LIST})
add_executable(${TEST_RUNTIME} main.cpp $<TARGET_OBJECTS:${TEST_OBJECT}>)

target_link_libraries(${TEST_OBJECT} PUBLIC ${LIB_NAME} Common Utils GTest::gtest)
target_link_libraries(${TEST_RUNTIME} PUBLIC ${LIB_NAME} Common Utils GTest::gtest)

target_link_libraries(${TEST_OBJECT} PRIVATE Threads::Threads)
target_link_libraries(${TEST_RUNTIME} PRIVATE Threads::Threads)

set_property(GLOBAL APPEND PROPERTY ORIGIN_TEST_OBJ_LIST $<TARGET_OBJECTS:${TEST_OBJECT}>)

# include(GoogleTest)
# gtest_discover_tests(${TEST_RUNTIME})

add_test(NAME ${TEST_RUNTIME} COMMAND ${TEST_RUNTIME})
set_tests_properties(${TEST_RUNTIME} PROPERTIES TIMEOUT 60)
