#ifndef ORIGIN_LOGGING_C_LOGGING_C_H
#define ORIGIN_LOGGING_C_LOGGING_C_H

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
#include <stdio.h>

typedef struct LoggerSt LoggerSt;
typedef struct SinkSt SinkSt;
typedef struct FormatterSt FormatterSt;
typedef struct TaskPoolSt TaskPoolSt;

#define ORIGIN_LOGGER_ROOT_NAME               "__root_logger__"
#define ORIGIN_FORMATTER_DEFAULT_PATTERN      "[%d][%L][%s:%#]: %v"
#define ORIGIN_THREAD_POOL_DEFAULT_CAPACITY   (4096)
#define ORIGIN_THREAD_POOL_DEFAULT_THREAD_CNT (1)

typedef enum {
    ORIGIN_LOG_LEVEL_TRACE = 0,
    ORIGIN_LOG_LEVEL_DEBUG,
    ORIGIN_LOG_LEVEL_INFO,
    ORIGIN_LOG_LEVEL_WARN,
    ORIGIN_LOG_LEVEL_ERROR,
    ORIGIN_LOG_LEVEL_FATAL,
    ORIGIN_LOG_LEVEL_OFF
} OriginLogLevel;

#pragma region Logger
LoggerSt *origin_create_sync_logger(const char *name, const SinkSt *const sinks[], uint32_t count);
LoggerSt *origin_create_async_logger(const char *name, const SinkSt *const sinks[], uint32_t count,
                                     const TaskPoolSt *taskPool);
LoggerSt *origin_create_async_logger_use_root_tp(const char *name, const SinkSt *const sinks[],
                                                 uint32_t count);
void origin_destroy_logger(LoggerSt *logger);

const char *origin_logger_name(const LoggerSt *logger);

void origin_logger_set_level(LoggerSt const *logger, OriginLogLevel level);
bool origin_logger_should_log(LoggerSt const *logger, OriginLogLevel level);
OriginLogLevel origin_logger_level(LoggerSt const *logger);

void origin_logger_flush_on(LoggerSt const *logger, OriginLogLevel level);
bool origin_logger_should_flush(LoggerSt const *logger, OriginLogLevel level);
OriginLogLevel origin_logger_flush_level(LoggerSt const *logger);

void origin_logger_set_pattern(const LoggerSt *logger, const char *pattern);
void origin_logger_set_formatter(const LoggerSt *logger, const FormatterSt *formatter);

void origin_logger_flush(const LoggerSt *logger);

void origin_logger_log(const LoggerSt *logger, const char *file, int line, const char *func,
                       OriginLogLevel level, const char *format, ...);
#pragma endregion

#pragma region Sink
SinkSt *origin_create_stdout_sink(FILE *file);
SinkSt *origin_create_basic_file_sink(const char *file, bool overwrite);
SinkSt *origin_create_daily_file_sink(const char *file, uint32_t hour, uint32_t minute,
                                      uint32_t maxFiles, bool overwrite);
SinkSt *origin_create_rotating_file_sink(const char *file, uint32_t maxFileSize, uint32_t maxFiles,
                                         bool rotateOnOpen);

void origin_detroy_sink(SinkSt *sink);

void origin_sink_set_level(const SinkSt *sink, OriginLogLevel level);
bool origin_sink_should_log(const SinkSt *sink, OriginLogLevel level);
OriginLogLevel origin_sink_level(const SinkSt *sink);

void origin_sink_set_pattern(const SinkSt *sink, const char *pattern);
void origin_sink_set_formatter(const SinkSt *sink, const FormatterSt *formatter);
#pragma endregion

#pragma region Formatter
FormatterSt *origin_create_pattern_formatter(const char *pattern);
void origin_destroy_formatter(FormatterSt *formatter);
#pragma endregion

#pragma region Task pool
TaskPoolSt *origin_create_task_pool(uint32_t capacity, uint32_t threadCnt);
void origin_destroy_task_pool(TaskPoolSt *taskPool);
#pragma endregion

#pragma region Root logger
LoggerSt *origin_root_logger();
void origin_set_root_logger(const LoggerSt *logger);

void origin_set_level(OriginLogLevel level);
bool origin_should_log(OriginLogLevel level);
OriginLogLevel origin_level();

void origin_flush_on(OriginLogLevel level);
bool origin_should_flush(OriginLogLevel level);
OriginLogLevel origin_flush_level();

void origin_set_pattern(const char *pattern);
void origin_set_formatter(const FormatterSt *formatter);

void origin_flush();

void origin_force_log(const char *file, int line, const char *func, OriginLogLevel level,
                      const char *format, ...);

void origin_log(const char *file, int line, const char *func, OriginLogLevel level,
                const char *format, ...);
#pragma endregion

#pragma region Registry
bool origin_register_logger(const LoggerSt *logger);
void origin_register_or_replace_logger(const LoggerSt *logger);
void origin_remove_logger(const char *name);
void origin_remove_all();
LoggerSt *origin_get_logger(const char *name);

void origin_init_root_task_pool(uint32_t capacity, uint32_t threadCnt);
TaskPoolSt *origin_root_task_pool();
#pragma endregion

#pragma region Logging manager
void origin_initialize_logger(LoggerSt const *logger, bool autoRegister);
void origin_set_level_all(OriginLogLevel level);
void origin_flush_on_all(OriginLogLevel level);
void origin_set_pattern_all(const char *pattern);
void origin_set_formatter_all(FormatterSt const *formatter);
void origin_flush_all();
void origin_shutdown();
#pragma endregion

#ifdef __cplusplus
}
#endif  // __cplusplus

#define ORIGIN_LOG_TRACE(fmt, ...) \
    origin_log(                    \
        __FILE__, __LINE__, __FUNCTION__, ORIGIN_LOG_LEVEL_TRACE, fmt __VA_OPT__(, ) __VA_ARGS__);
#define ORIGIN_LOG_DEBUG(fmt, ...) \
    origin_log(                    \
        __FILE__, __LINE__, __FUNCTION__, ORIGIN_LOG_LEVEL_DEBUG, fmt __VA_OPT__(, ) __VA_ARGS__);
#define ORIGIN_LOG_INFO(fmt, ...) \
    origin_log(                   \
        __FILE__, __LINE__, __FUNCTION__, ORIGIN_LOG_LEVEL_INFO, fmt __VA_OPT__(, ) __VA_ARGS__);
#define ORIGIN_LOG_WARN(fmt, ...) \
    origin_log(                   \
        __FILE__, __LINE__, __FUNCTION__, ORIGIN_LOG_LEVEL_WARN, fmt __VA_OPT__(, ) __VA_ARGS__);
#define ORIGIN_LOG_ERR(fmt, ...) \
    origin_log(                  \
        __FILE__, __LINE__, __FUNCTION__, ORIGIN_LOG_LEVEL_ERROR, fmt __VA_OPT__(, ) __VA_ARGS__);
#define ORIGIN_LOG_FATAL(fmt, ...) \
    origin_log(                    \
        __FILE__, __LINE__, __FUNCTION__, ORIGIN_LOG_LEVEL_FATAL, fmt __VA_OPT__(, ) __VA_ARGS__);

#endif  // ORIGIN_LOGGING_C_LOGGING_C_H
