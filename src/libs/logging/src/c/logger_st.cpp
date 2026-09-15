#include <cassert>
#include <cstdarg>
#include <cstring>
#include <exception>
#include <memory>
#include <string_view>

#include "c/internal/common_c.hpp"
#include "common/debug/debug_logger.h"
#include "internal/common.hpp"
#include "logging/c/logging_c.h"
#include "logging/loggers/async_logger.hpp"
#include "logging/loggers/sync_logger.hpp"
#include "utils/string_utils.h"

using namespace origin::logging;
using namespace origin::logging::c;
using namespace origin::string;

extern "C" {
LoggerSt *origin_create_sync_logger(const char *name, const SinkSt *const sinks[], uint32_t count)
{
    RETURN_VALUE_AND_ERROR_IF_TRUE(string_is_null_or_empty(name),
                                   nullptr,
                                   "Create sync logger failed. name nullptr or empty.");
    RETURN_VALUE_AND_ERROR_IF_TRUE((sinks == nullptr || count == 0),
                                   nullptr,
                                   "Create sync logger failed. sinks nullptr or count is 0.");

    try {
        return new struct LoggerSt(
            std::make_shared<SyncLogger>(name, sink_ptr_vector(sinks, count)));
    } catch (const std::exception &e) {
        ORIGIN_DEBUG_ERR("Create sync logger failed. Name: [{}]. Exception: {}", name, e.what());
        return nullptr;
    }
}

LoggerSt *origin_create_async_logger(const char *name, const SinkSt *const sinks[], uint32_t count,
                                     const TaskPoolSt *taskPool)
{
    RETURN_VALUE_AND_ERROR_IF_TRUE(string_is_null_or_empty(name),
                                   nullptr,
                                   "Create sync logger failed. name nullptr or empty.");

    RETURN_VALUE_AND_ERROR_IF_TRUE((sinks == nullptr || count == 0),
                                   nullptr,
                                   "Create async logger failed. sinks nullptr or count is 0.");

    try {
        if (taskPool) {
            return new struct LoggerSt(
                std::make_shared<AsyncLogger>(name, sink_ptr_vector(sinks, count), taskPool->ptr));
        } else {
            return new struct LoggerSt(
                std::make_shared<AsyncLogger>(name, sink_ptr_vector(sinks, count)));
        }
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
    RETURN_VALUE_AND_WARN_IF_TRUE(
        PTR_INVALID(logger), "", "Get logger name failed. {}", LOGGER_NULL_LOG);
    return logger->ptr->name().data();
}

void origin_logger_set_level(LoggerSt const *logger, LogLevelC level)
{
    RETURN_AND_ERROR_IF_TRUE(PTR_INVALID(logger), "Logger set level failed. {}", LOGGER_NULL_LOG);
    RETURN_AND_ERROR_IF_TRUE(log_level_c_invalid(level),
                             "Logger set level failed. Name: [{}]. {}",
                             logger->ptr->name(),
                             LOG_LEVEL_C_INVALID);
    logger->ptr->set_level(c_to_cpp_log_level(level));
}

bool origin_logger_should_log(LoggerSt const *logger, LogLevelC level)
{
    RETURN_VALUE_AND_WARN_IF_TRUE(
        PTR_INVALID(logger), false, "Logger should log failed. {}", LOGGER_NULL_LOG);
    RETURN_VALUE_AND_WARN_IF_TRUE(log_level_c_invalid(level),
                                  false,
                                  "Logger should log failed. Name: [{}] {}",
                                  logger->ptr->name(),
                                  LOG_LEVEL_C_INVALID);
    return logger->ptr->should_log(c_to_cpp_log_level(level));
}

LogLevelC origin_logger_level(LoggerSt const *logger)
{
    RETURN_VALUE_AND_WARN_IF_TRUE(
        PTR_INVALID(logger), ORIGIN_LOG_LEVEL_OFF, "Logger get level failed. {}", LOGGER_NULL_LOG);
    return cpp_to_c_log_level(logger->ptr->level());
}

void origin_logger_flush_on(LoggerSt const *logger, LogLevelC level)
{
    RETURN_AND_ERROR_IF_TRUE(PTR_INVALID(logger), "Logger flush on failed. {}", LOGGER_NULL_LOG);
    RETURN_AND_ERROR_IF_TRUE(log_level_c_invalid(level),
                             "Logger flush on failed. Name: [{}]. {}",
                             logger->ptr->name(),
                             LOG_LEVEL_C_INVALID)

    logger->ptr->flush_on(c_to_cpp_log_level(level));
}

bool origin_logger_should_flush(LoggerSt const *logger, LogLevelC level)
{
    RETURN_VALUE_AND_WARN_IF_TRUE(
        PTR_INVALID(logger), false, "Logger should flush failed. {}", LOGGER_NULL_LOG);
    RETURN_VALUE_AND_WARN_IF_TRUE(log_level_c_invalid(level),
                                  false,
                                  "Logger should flush failed. Name: [{}]. {}",
                                  logger->ptr->name(),
                                  LOG_LEVEL_C_INVALID);
    return logger->ptr->should_flush(c_to_cpp_log_level(level));
}

LogLevelC origin_logger_flush_level(LoggerSt const *logger)
{
    RETURN_VALUE_AND_WARN_IF_TRUE(
        PTR_INVALID(logger), ORIGIN_LOG_LEVEL_OFF, "Logger get flush level. {}", LOGGER_NULL_LOG);
    return cpp_to_c_log_level(logger->ptr->flush_level());
}

void origin_logger_set_pattern(const LoggerSt *logger, const char *pattern)
{
    RETURN_AND_ERROR_IF_TRUE(PTR_INVALID(logger), "Logger set pattern failed. {}", LOGGER_NULL_LOG);
    RETURN_AND_ERROR_IF_TRUE(string_is_null_or_empty(pattern),
                             "Logger set pattern failed. Name: [{}]. pattern nullptr or empty.",
                             logger->ptr->name());

    logger->ptr->set_pattern(pattern);
}

void origin_logger_set_formatter(const LoggerSt *logger, const FormatterSt *formatter)
{
    RETURN_AND_ERROR_IF_TRUE(
        PTR_INVALID(logger), "Logger set farmatter failed. {}", LOGGER_NULL_LOG);
    RETURN_AND_ERROR_IF_TRUE(PTR_INVALID(formatter),
                             "Logger set farmatter failed. Name: [{}]. {}",
                             logger->ptr->name(),
                             FORMATTER_NULL_LOG);

    logger->ptr->set_formatter(formatter->ptr->clone());
}

void origin_logger_flush(const LoggerSt *logger)
{
    RETURN_AND_WARN_IF_TRUE(PTR_INVALID(logger), "Logger flush failed. {}", LOGGER_NULL_LOG);
    logger->ptr->flush();
}

void origin_logger_log(const LoggerSt *logger, const char *file, int line, const char *func,
                       LogLevelC level, const char *format, ...)
{
    RETURN_AND_WARN_IF_TRUE(PTR_INVALID(logger), "Logger log failed. {}", LOGGER_NULL_LOG);
    RETURN_AND_WARN_IF_TRUE(log_level_c_invalid(level),
                            "Logger log failed. Name: [{}]. {}",
                            logger->ptr->name(),
                            LOG_LEVEL_C_INVALID);
    RETURN_AND_WARN_IF_TRUE(
        format == nullptr, "Logger log failed. Name: [{}]. format nullptr.", logger->ptr->name());
    va_list args;
    va_start(args, format);
    origin_log_it(logger->ptr, file, line, func, c_to_cpp_log_level(level), format, args);
    va_end(args);
}
}
