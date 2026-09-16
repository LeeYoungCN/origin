#include "logging/log_level.hpp"

#include <unordered_map>

namespace {
struct LogLvlStrInfo {
    const char* full;
    const char* abbr;
};

const std::unordered_map<origin::logging::LogLevel, LogLvlStrInfo> LOG_STR_MAP = {
    {origin::logging::LogLevel::TRACE, {.full = "TRACE", .abbr = "T"}},
    {origin::logging::LogLevel::DEBUG, {.full = "DEBUG", .abbr = "D"}},
    {origin::logging::LogLevel::INFO, {.full = "INFO", .abbr = "I"}},
    {origin::logging::LogLevel::WARN, {.full = "WARN", .abbr = "W"}},
    {origin::logging::LogLevel::ERR, {.full = "ERROR", .abbr = "E"}},
    {origin::logging::LogLevel::FATAL, {.full = "FATAL", .abbr = "F"}},
    {origin::logging::LogLevel::OFF, {.full = "OFF", .abbr = "O"}}};

}  // namespace

namespace origin::logging {

const char* log_level_full_name(LogLevel level)
{
    return LOG_STR_MAP.at(level).full;
}

const char* log_level_abbr_name(LogLevel level)
{
    return LOG_STR_MAP.at(level).abbr;
}
}  // namespace origin::logging
