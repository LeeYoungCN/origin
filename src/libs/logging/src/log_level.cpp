#include "logging/log_level.hpp"

#include <cstdint>
#include <unordered_map>

namespace {
struct LogLvlStrInfo {
    const char* full;
    const char* abbr;
};
}  // namespace

namespace origin::logging {

const char* log_level_string(LogLevel level, bool full)
{
    static std::unordered_map<LogLevel, LogLvlStrInfo> LOG_STR_MAP = {
        {LogLevel::TRACE, {.full = "TRACE", .abbr = "T"}},
        {LogLevel::DEBUG, {.full = "DEBUG", .abbr = "D"}},
        {LogLevel::INFO, {.full = "INFO", .abbr = "I"}},
        {LogLevel::WARN, {.full = "WARN", .abbr = "W"}},
        {LogLevel::ERR, {.full = "ERROR", .abbr = "E"}},
        {LogLevel::FATAL, {.full = "FATAL", .abbr = "F"}},
        {LogLevel::OFF, {.full = "OFF", .abbr = "O"}}};

    return full ? LOG_STR_MAP[level].full : LOG_STR_MAP[level].abbr;
}

LOGGING_API int32_t diff_log_level(LogLevel a, LogLevel b)
{
    return static_cast<int32_t>(a) - static_cast<int32_t>(b);
}
}  // namespace origin::logging
