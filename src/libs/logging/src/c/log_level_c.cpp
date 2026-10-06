#include <cstdint>

#include "internal/common_c.hpp"
#include "logging/c/logging_c.h"
#include "logging/log_level.hpp"

using namespace origin::logging;
using namespace c_api;

static_assert(static_cast<uint32_t>(LogLevel::TRACE) == LOG_LEVEL_C_TRACE);
static_assert(static_cast<uint32_t>(LogLevel::DEBUG) == LOG_LEVEL_C_DEBUG);
static_assert(static_cast<uint32_t>(LogLevel::INFO) == LOG_LEVEL_C_INFO);
static_assert(static_cast<uint32_t>(LogLevel::WARN) == LOG_LEVEL_C_WARN);
static_assert(static_cast<uint32_t>(LogLevel::ERR) == LOG_LEVEL_C_ERROR);
static_assert(static_cast<uint32_t>(LogLevel::FATAL) == LOG_LEVEL_C_FATAL);
static_assert(static_cast<uint32_t>(LogLevel::OFF) == LOG_LEVEL_C_OFF);

extern "C" {
const char* origin_log_level_full_name(const LogLevelC level)
{
    return log_level_full_name(c_to_cpp_log_level(level));
}

const char* origin_log_level_abbr_name(const LogLevelC level)
{
    return log_level_abbr_name(c_to_cpp_log_level(level));
}
}
