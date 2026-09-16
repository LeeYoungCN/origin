#include "internal/common_c.hpp"
#include "logging/c/logging_c.h"
#include "logging/log_level.hpp"

using namespace origin::logging;
static_assert(static_cast<uint32_t>(origin::logging::LogLevel::TRACE) == ORIGIN_LOG_LEVEL_TRACE);
static_assert(static_cast<uint32_t>(origin::logging::LogLevel::DEBUG) == ORIGIN_LOG_LEVEL_DEBUG);
static_assert(static_cast<uint32_t>(origin::logging::LogLevel::INFO) == ORIGIN_LOG_LEVEL_INFO);
static_assert(static_cast<uint32_t>(origin::logging::LogLevel::WARN) == ORIGIN_LOG_LEVEL_WARN);
static_assert(static_cast<uint32_t>(origin::logging::LogLevel::ERR) == ORIGIN_LOG_LEVEL_ERROR);
static_assert(static_cast<uint32_t>(origin::logging::LogLevel::FATAL) == ORIGIN_LOG_LEVEL_FATAL);
static_assert(static_cast<uint32_t>(origin::logging::LogLevel::OFF) == ORIGIN_LOG_LEVEL_OFF);

extern "C" {
const char* origin_log_level_full_name(LogLevelC level)
{
    return origin::logging::log_level_string(c::c_to_cpp_log_level(level), true);
}

const char* origin_log_level_abbr_name(LogLevelC level)
{
    return origin::logging::log_level_string(c::c_to_cpp_log_level(level), false);
}
}
