set(TEST_RUNTIME test_${LIB_NAME})
set(TEST_OBJECT  test_${LIB_NAME}_obj)

# 显式源文件清单：新增/删除测试文件时同步改这里。
set(SRC_LIST
    # 测试辅助
    src/detail/common.cpp
    src/detail/mock_sinks/base_test_sink.cpp
    src/detail/mock_sinks/log_content_buffer_sink.cpp
    src/detail/mock_sinks/tid_msg_map_sink.cpp
    # C API
    src/test_c_api/test_async_logger_st.cpp
    src/test_c_api/test_logger_st_api.cpp
    src/test_c_api/test_registry_c.cpp
    src/test_c_api/test_root_logger_c.cpp
    src/test_c_api/test_sink_st_api.cpp
    src/test_c_api/test_sync_logger_st.cpp
    # loggers
    src/test_loggers/test_async_logger.cpp
    src/test_loggers/test_logger_api.cpp
    src/test_loggers/test_root_logger.cpp
    src/test_loggers/test_sync_logger.cpp
    # sinks
    src/test_sinks/test_basic_file_sink.cpp
    src/test_sinks/test_daily_file_sink.cpp
    src/test_sinks/test_rotating_file_sink.cpp
    src/test_sinks/test_sink_api.cpp
    # 顶层用例
    src/test_logging_mpmc.cpp
    src/test_pattern_formatter.cpp
    src/test_registry.cpp
)

add_library(${TEST_OBJECT} OBJECT ${SRC_LIST})
add_executable(${TEST_RUNTIME} main.cpp $<TARGET_OBJECTS:${TEST_OBJECT}>)

target_include_directories(${TEST_OBJECT}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src  # 编译时路径
)

target_include_directories(${TEST_RUNTIME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src  # 编译时路径
)

target_link_libraries(${TEST_OBJECT} PUBLIC ${LIB_NAME} GTest::gtest)
target_link_libraries(${TEST_RUNTIME} PUBLIC ${LIB_NAME} GTest::gtest)

target_link_libraries(${TEST_OBJECT} PRIVATE Threads::Threads)
target_link_libraries(${TEST_RUNTIME} PRIVATE Threads::Threads)

set_property(GLOBAL APPEND PROPERTY ORIGIN_TEST_OBJ_LIST $<TARGET_OBJECTS:${TEST_OBJECT}>)

# include(GoogleTest)
# gtest_discover_tests(${TEST_RUNTIME})

add_test(NAME ${TEST_RUNTIME} COMMAND ${TEST_RUNTIME})
set_tests_properties(${TEST_RUNTIME} PROPERTIES TIMEOUT 60)
