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
    src/test_logging_api.cpp
)

origin_add_component_tests(${LIB_NAME} INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
