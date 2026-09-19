#include "logging/c/logging_c.h"

#include <cstdarg>
#include <cstdint>

#include "c/internal/common_c.hpp"
#include "common/base/singleton.h"
#include "common/debug/debug_logger.h"
#include "internal/common.hpp"
#include "internal/registry.hpp"

using namespace origin::logging;
using namespace origin::logging::c_api;

#define ROOT_LOGGER (INST(Registry).root_logger())

extern "C" {
LoggerSt *origin_root_logger()
{
    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_WARN("Root logger nullptr.");
        return nullptr;
    }
    return new struct LoggerSt(ROOT_LOGGER);
}

void origin_set_root_logger(const LoggerSt *logger)
{
    if (logger == nullptr) {
        ORIGIN_DEBUG_ERR("Set root logger failed. logger nullptr.")
        return;
    }
    REGISTRY.set_root_logger(logger->ptr);
}

void origin_set_level(LogLevelC level)
{
    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_ERR("Root logger set level failed. {}", LOG_LEVEL_C_INVALID_LOG);
        return;
    }
    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_ERR("Root logger set level failed. {}", ROOT_LOGGER_NULL_LOG);
        return;
    }
    ROOT_LOGGER->set_level(c_to_cpp_log_level(level));
}

bool origin_should_log(const LogLevelC level)
{
    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_ERR("Root logger should log failed. {}", LOG_LEVEL_C_INVALID_LOG);
        return false;
    }
    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_ERR("Root logger should log failed. {}", ROOT_LOGGER_NULL_LOG);
        return false;
    }
    return ROOT_LOGGER->should_log(c_to_cpp_log_level(level));
}

LogLevelC origin_level()
{
    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_WARN("Root logger get log level failed. {}", ROOT_LOGGER_NULL_LOG);
        return ORIGIN_LOG_LEVEL_OFF;
    }
    return cpp_to_c_log_level(ROOT_LOGGER->level());
}

void origin_flush_on(const LogLevelC level)
{
    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_ERR("Root logger flush on failed. {}", LOG_LEVEL_C_INVALID_LOG);
        return;
    }

    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_ERR("Root logger flush on failed. {}", ROOT_LOGGER_NULL_LOG);
        return;
    }

    ROOT_LOGGER->flush_on(c_to_cpp_log_level(level));
}

bool origin_should_flush(const LogLevelC level)
{
    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_ERR("Root logger should flush failed. {}", LOG_LEVEL_C_INVALID_LOG);
        return false;
    }
    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_ERR("Root logger should flush failed. {}", ROOT_LOGGER_NULL_LOG);
        return false;
    }
    return ROOT_LOGGER->should_flush(c_to_cpp_log_level(level));
}

LogLevelC origin_flush_level()
{
    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_ERR("Root logger get flush level failed. {}", ROOT_LOGGER_NULL_LOG);
        return ORIGIN_LOG_LEVEL_OFF;
    }
    return cpp_to_c_log_level(ROOT_LOGGER->flush_level());
}

void origin_set_pattern(const char *pattern)
{
    if (pattern == nullptr) {
        ORIGIN_DEBUG_ERR("Root logger set pattern failed. pattern nullptr.");
        return;
    }
    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_ERR("Root logger set pattern failed. {}", ROOT_LOGGER_NULL_LOG);
        return;
    }
    return ROOT_LOGGER->set_pattern(pattern);
}

void origin_set_formatter(const FormatterSt *formatter)
{
    if (formatter == nullptr) {
        ORIGIN_DEBUG_ERR("Root logger set formatter failed. formatter nullptr.");
        return;
    }
    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_ERR("Root logger set formatter failed. {}", ROOT_LOGGER_NULL_LOG);
        return;
    }
    return ROOT_LOGGER->set_formatter(formatter->ptr);
}

void origin_flush()
{
    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_ERR("Root logger flush failed. {}", ROOT_LOGGER_NULL_LOG);
        return;
    }
    ROOT_LOGGER->flush();
}

void origin_log(const char *file, const int line, const char *func, LogLevelC level, const char *format,
                ...)
{
    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_WARN("Root logger log failed. {}", LOG_LEVEL_C_INVALID_LOG);
        return;
    }

    if (format == nullptr) {
        ORIGIN_DEBUG_WARN("Root logger log failed. format nullptr.");
        return;
    }

    if (!ROOT_LOGGER) {
        ORIGIN_DEBUG_WARN("Root logger log failed. {}", ROOT_LOGGER_NULL_LOG);
        return;
    }

    va_list args;
    va_start(args, format);
    origin_log_it(ROOT_LOGGER, file, line, func, c_to_cpp_log_level(level), format, args);
    va_end(args);
}

bool origin_register_logger(const LoggerSt *logger)
{
    if (logger == nullptr) {
        ORIGIN_DEBUG_ERR("Register logger failed. logger nullptr.");
        return false;
    }
    return REGISTRY.register_logger(logger->ptr);
}

void origin_register_or_replace_logger(const LoggerSt *logger)
{
    if (logger == nullptr) {
        ORIGIN_DEBUG_ERR("Register logger failed. logger nullptr.");
        return;
    }

    REGISTRY.register_or_replace_logger(logger->ptr);
}

void origin_remove_logger(const char *name)
{
    if (name == nullptr) {
        ORIGIN_DEBUG_ERR("Remove logger failed. name nullptr.");
        return;
    }
    REGISTRY.remove_logger(name);
}

void origin_remove_all()
{
    REGISTRY.remove_all();
}

LoggerSt *origin_get_logger(const char *name)
{
    if (name == nullptr) {
        ORIGIN_DEBUG_ERR("Get logger failed. name nullptr.");
        return nullptr;
    }
    const auto logger = REGISTRY.get_logger(name);
    if (logger == nullptr) {
        return nullptr;
    }
    return new LoggerSt(logger);
}

void origin_init_root_task_pool(uint32_t capacity, uint32_t threadCnt)
{
    REGISTRY.init_root_task_pool(capacity, threadCnt);
}

void origin_set_root_task_pool(const TaskPoolSt *taskPool)
{
    if (taskPool == nullptr) {
        ORIGIN_DEBUG_ERR("Set root task pool failed. taskPool nullptr.");
        return;
    }
    REGISTRY.set_root_task_pool(taskPool->ptr);
}

TaskPoolSt *origin_root_task_pool()
{
    const auto taskPool = REGISTRY.root_task_pool();
    if (taskPool == nullptr) {
        return nullptr;
    }
    return new struct TaskPoolSt(taskPool);
}

void origin_initialize_logger(LoggerSt const *logger, bool autoRegister)
{
    if (logger == nullptr) {
        ORIGIN_DEBUG_ERR("Initialize logger failed. logger nullptr.");
        return;
    }
    REGISTRY.initialize_logger(logger->ptr, autoRegister);
}

void origin_set_level_all(LogLevelC level)
{
    REGISTRY.set_level_all(c_to_cpp_log_level(level));
}

void origin_flush_on_all(const LogLevelC level)
{
    REGISTRY.flush_on_all(c_to_cpp_log_level(level));
}

void origin_set_pattern_all(const char *pattern)
{
    RETURN_AND_LOG_IF_PTR_NULL(pattern, "Set pattern all failed.");
    REGISTRY.set_pattern_all(pattern);
}

void origin_set_formatter_all(const FormatterSt *formatter)
{
    RETURN_AND_LOG_IF_PTR_NULL(formatter, "Set formatter all failed.");
    RETURN_AND_LOG_IF_PTR_NULL(formatter->ptr, "Set formatter all failed.");
    REGISTRY.set_formatter_all(formatter->ptr->clone());
}

void origin_flush_all()
{
    REGISTRY.flush_all();
}

void origin_shutdown()
{
    REGISTRY.shutdown();
}
}
