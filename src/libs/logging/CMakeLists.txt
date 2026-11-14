set(LIB_NAME Logging)

# 显式源文件清单：新增/删除源文件时同步改这里。
set(SRC_LIST
    # C API
    src/c/internal/common_c.cpp
    src/c/formatter_st.cpp
    src/c/logger_st.cpp
    src/c/log_level_c.cpp
    src/c/logging_c.cpp
    src/c/sink_st.cpp
    src/c/task_pool_st.cpp
    # formatters
    src/formatters/formatter_base.cpp
    src/formatters/pattern_formatter.cpp
    src/formatters/internal/pattern_formatter_impl.cpp
    # internal
    src/internal/common.cpp
    src/internal/registry.cpp
    src/internal/task_pool.cpp
    # loggers
    src/loggers/async_logger.cpp
    src/loggers/logger_base.cpp
    src/loggers/sync_logger.cpp
    src/loggers/internal/async_logger_impl.cpp
    src/loggers/internal/logger_impl_base.cpp
    src/loggers/internal/sync_logger_impl.cpp
    # sinks
    src/sinks/basic_file_sink.cpp
    src/sinks/daily_file_sink.cpp
    src/sinks/rotating_file_sink.cpp
    src/sinks/sink_base.cpp
    src/sinks/stderr_sink.cpp
    src/sinks/stdout_sink.cpp
    src/sinks/internal/basic_file_sink_impl.cpp
    src/sinks/internal/daily_file_sink_impl.cpp
    src/sinks/internal/rotating_file_sink_impl.cpp
    src/sinks/internal/rotating_file_sink_impl_base.cpp
    src/sinks/internal/sink_impl_base.cpp
    src/sinks/internal/stream_sink_impl.cpp
    # root
    src/logging.cpp
    src/log_level.cpp
    src/log_msg.cpp
)
add_library(${LIB_NAME} SHARED ${SRC_LIST})
target_compile_definitions(${LIB_NAME} PRIVATE LOGGING_BUILD_SHARED)

origin_add_component(${LIB_NAME})

target_link_libraries(${LIB_NAME} PUBLIC Common Container Utils)


if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
