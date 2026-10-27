#ifndef ORIGIN_LOGGING_LOG_MSG_HPP
#define ORIGIN_LOGGING_LOG_MSG_HPP

#include <string>
#include <string_view>

#include "origin/common/types/date_time_types.h"
#include "origin/common/types/process_types.h"
#include "origin/common/types/thread_types.h"
#include "origin/logging/detail/logging_api.h"
#include "origin/logging/log_level.hpp"
#include "origin/logging/log_source.hpp"

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

LOGGING_API void init_log_msg(const LogSource& source, std::string_view loggerName, LogLevel level,
                              std::string_view message, LogMsg& msg);
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_LOG_MSG_HPP
