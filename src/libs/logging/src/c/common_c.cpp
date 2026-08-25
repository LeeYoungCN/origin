#include "c/common_c.h"

#include <cstdint>
#include <memory>
#include <vector>

#include "logging/c/logging_c.h"
#include "logging/log_level.h"

namespace origin::logging::c {
LogLevel c_to_cpp_log_level(OriginLogLevel level)
{
    switch (level) {
        case ORIGIN_LOG_LEVEL_DEBUG:
            return LogLevel::DEBUG;
        case ORIGIN_LOG_LEVEL_INFO:
            return LogLevel::INFO;
        case ORIGIN_LOG_LEVEL_WARN:
            return LogLevel::WARN;
        case ORIGIN_LOG_LEVEL_ERROR:
            return LogLevel::ERR;
        case ORIGIN_LOG_LEVEL_FATAL:
            return LogLevel::FATAL;
        case ORIGIN_LOG_LEVEL_OFF:
        default:
            return LogLevel::OFF;
    }
}

OriginLogLevel cpp_to_c_log_level(LogLevel level)
{
    switch (level) {
        case LogLevel::DEBUG:
            return ORIGIN_LOG_LEVEL_DEBUG;
        case LogLevel::INFO:
            return ORIGIN_LOG_LEVEL_INFO;
        case LogLevel::WARN:
            return ORIGIN_LOG_LEVEL_WARN;
        case LogLevel::ERR:
            return ORIGIN_LOG_LEVEL_ERROR;
        case LogLevel::FATAL:
            return ORIGIN_LOG_LEVEL_FATAL;
        case LogLevel::OFF:
        default:
            return ORIGIN_LOG_LEVEL_OFF;
    }
}

std::vector<std::shared_ptr<Sink>> sink_ptr_vector(const SinkSt *const sinks[], uint32_t sinkCnt)
{
    std::vector<std::shared_ptr<Sink>> sinkPtrs;
    sinkPtrs.reserve(sinkCnt);
    for (uint32_t i = 0; i < sinkCnt; ++i) {
        sinkPtrs.emplace_back(sinks[i]->ptr);
    }
    return sinkPtrs;
}

void origin_force_log_it(const std::shared_ptr<Logger> &logger, const char *file, int line,
                         const char *func, LogLevel level, const char *format, va_list args)
{
    logger->force_log(
        LogSource(file, line, func), level, origin::string::va_list_to_string(format, args));
}

void origin_log_it(const std::shared_ptr<Logger> &logger, const char *file, int line,
                   const char *func, LogLevel level, const char *format, va_list args)
{
    if (logger->should_log(level)) {
        origin_force_log_it(logger, file, line, func, level, format, args);
    }
}
}  // namespace origin::logging::c
