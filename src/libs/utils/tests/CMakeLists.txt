set(TEST_RUNTIME test_${LIB_NAME})
set(TEST_OBJECT  test_${LIB_NAME}_obj)

# 显式源文件清单：新增/删除测试文件时同步改这里。
set(SRC_LIST
    # date_time
    src/test_date_time_utils/test_date_time_utils.cpp
    src/test_date_time_utils/test_date_time_utils_common.cpp
    src/test_date_time_utils/test_date_time_utils_string_date_time.cpp
    src/test_date_time_utils/test_date_time_utils_string_timestamp.cpp
    # filesystem
    src/test_filesystem_utils/test_filesystem_utils_dir.cpp
    src/test_filesystem_utils/test_filesystem_utils_file.cpp
    src/test_filesystem_utils/test_filesystem_utils_path.cpp
    src/test_filesystem_utils/test_file_writer.cpp
    # thread
    src/test_thread_utils/test_thread_utils.cpp
)

add_library(${TEST_OBJECT} OBJECT ${SRC_LIST})
add_executable(${TEST_RUNTIME} main.cpp $<TARGET_OBJECTS:${TEST_OBJECT}>)

target_link_libraries(${TEST_OBJECT} PUBLIC ${LIB_NAME} GTest::gtest)
target_link_libraries(${TEST_RUNTIME} PUBLIC ${LIB_NAME} GTest::gtest)

target_link_libraries(${TEST_OBJECT} PRIVATE Threads::Threads)
target_link_libraries(${TEST_RUNTIME} PRIVATE Threads::Threads)

set_property(GLOBAL APPEND PROPERTY ORIGIN_TEST_OBJ_LIST $<TARGET_OBJECTS:${TEST_OBJECT}>)

# include(GoogleTest)
# gtest_discover_tests(${TEST_RUNTIME})


add_test(NAME ${TEST_RUNTIME} COMMAND ${TEST_RUNTIME})
set_tests_properties(${TEST_RUNTIME} PROPERTIES TIMEOUT 60)
