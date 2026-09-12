#pragma once
#ifndef ORIGIN_LOGGING_LOG_MSG_HPP
#define ORIGIN_LOGGING_LOG_MSG_HPP

#include <string>
#include <string_view>

#include "common/types/date_time_types.h"
#include "common/types/process_types.h"
#include "common/types/thread_types.h"
#include "logging/log_level.hpp"
#include "logging/log_source.hpp"
#include "logging/logging_api.h"

namespace origin::logging {
struct LogMsg {
    std::string loggerName;
    LogLevel level{LogLevel::DEBUG};
    std::string data;
    LogSource source;
    TimestampMs timestamp{0};
    ThreadId threadId{0};
    ProcessId procId{0};
};

LOGGING_API LogMsg create_log_msg(const LogSource& source, std::string_view loggerName,
                                  LogLevel level, std::string_view message);

LOGGING_API void init_log_msg(LogMsg& msg, const LogSource& source, std::string_view loggerName,
                              LogLevel level, std::string_view message);
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_LOG_MSG_HPP
