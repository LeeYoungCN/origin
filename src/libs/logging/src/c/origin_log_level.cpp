#include <cstdint>
#include "internal/common_c.hpp"
#include "logging/c/logging_c.h"
#include "logging/log_level.hpp"

using namespace origin::logging;
using namespace c_api;

static_assert(static_cast<uint32_t>(LogLevel::TRACE) == ORIGIN_LOG_LEVEL_TRACE);
static_assert(static_cast<uint32_t>(LogLevel::DEBUG) == ORIGIN_LOG_LEVEL_DEBUG);
static_assert(static_cast<uint32_t>(LogLevel::INFO) == ORIGIN_LOG_LEVEL_INFO);
static_assert(static_cast<uint32_t>(LogLevel::WARN) == ORIGIN_LOG_LEVEL_WARN);
static_assert(static_cast<uint32_t>(LogLevel::ERR) == ORIGIN_LOG_LEVEL_ERROR);
static_assert(static_cast<uint32_t>(LogLevel::FATAL) == ORIGIN_LOG_LEVEL_FATAL);
static_assert(static_cast<uint32_t>(LogLevel::OFF) == ORIGIN_LOG_LEVEL_OFF);

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
