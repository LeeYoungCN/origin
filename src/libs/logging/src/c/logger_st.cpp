#include <cassert>
#include <cstdarg>
#include <exception>
#include <memory>
#include <string_view>

#include "c/internal/common_c.hpp"
#include "common/debug/debug_logger.h"
#include "logging/c/logging_c.h"
#include "logging/loggers/async_logger.hpp"
#include "logging/loggers/sync_logger.hpp"

using namespace origin::logging;
using namespace origin::logging::c;
using namespace origin::string;

extern "C" {
LoggerSt *origin_create_sync_logger(const char *name, const SinkSt *const sinks[], uint32_t count)
{
    if (name == nullptr) {
        ORIGIN_DEBUG_ERR("Create sync logger failed. name nullptr.")
        return nullptr;
    }

    if ((sinks == nullptr || count == 0)) {
        ORIGIN_DEBUG_ERR("Create sync logger failed. sinks nullptr or count is 0.")
        return nullptr;
    }
    try {
        return new LoggerSt(std::make_shared<SyncLogger>(name, sink_ptr_vector(sinks, count)));
    } catch (const std::exception &e) {
        ORIGIN_DEBUG_ERR("Create sync logger failed. Name: [{}]. Exception: {}", name, e.what());
        return nullptr;
    }
}

LoggerSt *origin_create_async_logger(const char *name, const SinkSt *const sinks[], uint32_t count,
                                     const TaskPoolSt *taskPool)
{
    if (name == nullptr) {
        ORIGIN_DEBUG_ERR("Create async logger failed. name nullptr.")
        return nullptr;
    }

    if ((sinks == nullptr || count == 0)) {
        ORIGIN_DEBUG_ERR("Create async logger failed. sinks nullptr or count is 0.")
        return nullptr;
    }

    try {
        if (taskPool) {
            return new LoggerSt(
                std::make_shared<AsyncLogger>(name, sink_ptr_vector(sinks, count), taskPool->ptr));
        }
        return new struct LoggerSt(
            std::make_shared<AsyncLogger>(name, sink_ptr_vector(sinks, count)));
    } catch (const std::exception &e) {
        ORIGIN_DEBUG_ERR("Create async logger failed. Name: [{}]. Exception: {}", name, e.what());
        return nullptr;
    }
}

void origin_destroy_logger(LoggerSt *logger)
{
    if (logger != nullptr) {
        if (logger->ptr != nullptr) {
            ORIGIN_DEBUG_DBG("Release logger. Name: [{}], UseCnt: {}.",
                             logger->ptr->name(),
                             logger->ptr.use_count());
            logger->ptr.reset();
        }
        delete logger;
    }
}

const char *origin_logger_name(const LoggerSt *logger)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_WARN("Get logger name failed. {}", LOGGER_NULL_LOG);
        return "";
    }
    return logger->ptr->name().data();
}

void origin_logger_set_level(LoggerSt const *logger, LogLevelC level)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_ERR("Logger set level failed. {}", LOGGER_NULL_LOG);
        return;
    }

    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_ERR("Logger set level failed. Name: [{}]. {}",
                         logger->ptr->name(),
                         LOG_LEVEL_C_INVALID_LOG);
        return;
    }
    logger->ptr->set_level(c_to_cpp_log_level(level));
}

bool origin_logger_should_log(LoggerSt const *logger, LogLevelC level)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_WARN("Logger should log failed. {}", LOGGER_NULL_LOG);
        return false;
    }

    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_WARN("Logger should log failed. Name: [{}]. {}",
                          logger->ptr->name(),
                          LOG_LEVEL_C_INVALID_LOG);
        return false;
    }
    return logger->ptr->should_log(c_to_cpp_log_level(level));
}

LogLevelC origin_logger_level(LoggerSt const *logger)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_WARN("Logger get level failed. {}", LOGGER_NULL_LOG);
        return ORIGIN_LOG_LEVEL_OFF;
    }
    return cpp_to_c_log_level(logger->ptr->level());
}

void origin_logger_flush_on(LoggerSt const *logger, LogLevelC level)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_ERR("Logger flush on failed. {}", LOGGER_NULL_LOG);
        return;
    }

    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_ERR(
            "Logger flush on failed. Name: [{}]. {}", logger->ptr->name(), LOG_LEVEL_C_INVALID_LOG);
        return;
    }

    logger->ptr->flush_on(c_to_cpp_log_level(level));
}

bool origin_logger_should_flush(LoggerSt const *logger, LogLevelC level)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_WARN("Logger should flush failed. {}", LOGGER_NULL_LOG);
        return false;
    }

    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_WARN("Logger should flush failed. Name: [{}]. {}",
                          logger->ptr->name(),
                          LOG_LEVEL_C_INVALID_LOG);
        return false;
    }
    return logger->ptr->should_flush(c_to_cpp_log_level(level));
}

LogLevelC origin_logger_flush_level(LoggerSt const *logger)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_WARN("Logger get flush level. {}", LOGGER_NULL_LOG);
        return ORIGIN_LOG_LEVEL_OFF;
    }
    return cpp_to_c_log_level(logger->ptr->flush_level());
}

void origin_logger_set_pattern(const LoggerSt *logger, const char *pattern)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_ERR("Logger set pattern failed. {}", LOGGER_NULL_LOG);
        return;
    }

    if (pattern == nullptr) {
        ORIGIN_DEBUG_ERR("Logger set pattern failed. Name: [{}]. pattern nullptr.",
                         logger->ptr->name());
        return;
    }

    logger->ptr->set_pattern(pattern);
}

void origin_logger_set_formatter(const LoggerSt *logger, const FormatterSt *formatter)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_ERR("Logger set formatter failed. {}", LOGGER_NULL_LOG);
        return;
    }

    if (formatter == nullptr) {
        ORIGIN_DEBUG_ERR("Logger set formatter failed. Name: [{}]. pattern nullptr.",
                         logger->ptr->name());
        return;
    }
    logger->ptr->set_formatter(formatter->ptr);
}

void origin_logger_flush(const LoggerSt *logger)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_ERR("Logger flush failed. {}", LOGGER_NULL_LOG);
        return;
    }
    logger->ptr->flush();
}

void origin_logger_log(const LoggerSt *logger, const char *file, int line, const char *func,
                       LogLevelC level, const char *format, ...)
{
    if (PTR_INVALID(logger)) {
        ORIGIN_DEBUG_ERR("Logger log failed. {}", LOGGER_NULL_LOG);
        return;
    }
    if (log_level_c_invalid(level)) {
        ORIGIN_DEBUG_ERR(
            "Logger log failed. Name: [{}]. {}", logger->ptr->name(), LOG_LEVEL_C_INVALID_LOG);
        return;
    }
    if (format == nullptr) {
        ORIGIN_DEBUG_ERR("Logger log failed. Name: [{}]. format nullptr.", logger->ptr->name());
        return;
    }
    va_list args;
    va_start(args, format);
    origin_log_it(logger->ptr, file, line, func, c_to_cpp_log_level(level), format, args);
    va_end(args);
}
}
