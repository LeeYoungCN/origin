#include "logging/log_level.hpp"

#include <unordered_map>

#include "logging/detail/constants.h"

namespace {
struct LogLvlStrInfo {
    const char* full;
    const char* abbr;
};

const std::unordered_map<origin::logging::LogLevel, LogLvlStrInfo> LOG_STR_MAP = {
    {origin::logging::LogLevel::TRACE,
     {.full = LOG_LEVEL_FULL_NAME_TRACE, .abbr = LOG_LEVEL_ABBR_NAME_TRACE}},
    {origin::logging::LogLevel::DEBUG,
     {.full = LOG_LEVEL_FULL_NAME_DEBUG, .abbr = LOG_LEVEL_ABBR_NAME_DEBUG}},
    {origin::logging::LogLevel::INFO,
     {.full = LOG_LEVEL_FULL_NAME_INFO, .abbr = LOG_LEVEL_ABBR_NAME_INFO}},
    {origin::logging::LogLevel::WARN,
     {.full = LOG_LEVEL_FULL_NAME_WARN, .abbr = LOG_LEVEL_ABBR_NAME_WARN}},
    {origin::logging::LogLevel::ERR,
     {.full = LOG_LEVEL_FULL_NAME_ERROR, .abbr = LOG_LEVEL_ABBR_NAME_ERROR}},
    {origin::logging::LogLevel::FATAL,
     {.full = LOG_LEVEL_FULL_NAME_FATAL, .abbr = LOG_LEVEL_ABBR_NAME_FATAL}},
    {origin::logging::LogLevel::OFF,
     {.full = LOG_LEVEL_FULL_NAME_OFF, .abbr = LOG_LEVEL_ABBR_NAME_OFF}}};

}  // namespace

namespace origin::logging {

const char* log_level_full_name(const LogLevel level)
{
    return LOG_STR_MAP.at(level).full;
}

const char* log_level_abbr_name(const LogLevel level)
{
    return LOG_STR_MAP.at(level).abbr;
}
}  // namespace origin::logging
