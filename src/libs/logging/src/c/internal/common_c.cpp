#include "c/internal/common_c.hpp"

#include <cstdarg>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include "logging/c/logging_c.h"
#include "logging/log_level.hpp"
#include "logging/log_source.hpp"
#include "logging/loggers/logger.hpp"
#include "logging/sinks/sink.hpp"
#include "utils/string_utils.h"

namespace origin::logging::c_api {
bool log_level_c_invalid(const LogLevelC level)
{
    return (level > LOG_LEVEL_C_OFF || level < LOG_LEVEL_C_TRACE);
}

LogLevel c_to_cpp_log_level(const LogLevelC level)
{
    switch (level) {
        case LOG_LEVEL_C_TRACE:
            return LogLevel::TRACE;
        case LOG_LEVEL_C_DEBUG:
            return LogLevel::DEBUG;
        case LOG_LEVEL_C_INFO:
            return LogLevel::INFO;
        case LOG_LEVEL_C_WARN:
            return LogLevel::WARN;
        case LOG_LEVEL_C_ERROR:
            return LogLevel::ERR;
        case LOG_LEVEL_C_FATAL:
            return LogLevel::FATAL;
        case LOG_LEVEL_C_OFF:
            return LogLevel::OFF;
        default:
            throw std::invalid_argument(LOG_LEVEL_C_INVALID_LOG.data());
    }
}

LogLevelC cpp_to_c_log_level(const LogLevel level)
{
    switch (level) {
        case LogLevel::TRACE:
            return LOG_LEVEL_C_TRACE;
        case LogLevel::DEBUG:
            return LOG_LEVEL_C_DEBUG;
        case LogLevel::INFO:
            return LOG_LEVEL_C_INFO;
        case LogLevel::WARN:
            return LOG_LEVEL_C_WARN;
        case LogLevel::ERR:
            return LOG_LEVEL_C_ERROR;
        case LogLevel::FATAL:
            return LOG_LEVEL_C_FATAL;
        case LogLevel::OFF:
            return LOG_LEVEL_C_OFF;
        default:
            throw std::invalid_argument("LogLevel invalid.");
    }
}

std::vector<std::shared_ptr<Sink>> sink_ptr_vector(const SinkSt *const sinks[],
                                                   const uint32_t sinkCnt)
{
    std::vector<std::shared_ptr<Sink>> sinkPtrs;
    if (sinks != nullptr && sinkCnt > 0) {
        sinkPtrs.reserve(sinkCnt);
        for (uint32_t i = 0; i < sinkCnt; ++i) {
            if (sinks[i] != nullptr) {
                sinkPtrs.emplace_back(sinks[i]->ptr);
            } else {
                sinkPtrs.emplace_back(nullptr);
            }
        }
    }
    return sinkPtrs;
}

void origin_force_log_it(const std::shared_ptr<Logger> &logger, const char *file, const int line,
                         const char *func, const LogLevel level, const char *format, va_list args)
{
    logger->force_log(
        LogSource(file, line, func), level, origin::string::va_list_to_string(format, args));
}

void origin_log_it(const std::shared_ptr<Logger> &logger, const char *file, const int line,
                   const char *func, const LogLevel level, const char *format, va_list args)
{
    if (logger->should_log(level)) {
        origin_force_log_it(logger, file, line, func, level, format, args);
    }
}
}  // namespace origin::logging::c_api
