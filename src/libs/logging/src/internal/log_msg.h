#pragma once
#ifndef ORIGIN_LOGGING_INTERNAL_LOG_MSG_H
#define ORIGIN_LOGGING_INTERNAL_LOG_MSG_H

#include <string>
#include <string_view>

#include "common/types/date_time_types.h"
#include "common/types/process_types.h"
#include "common/types/thread_types.h"
#include "logging/log_level.h"
#include "logging/log_source.h"

namespace origin::logging {
struct LogMsg {
    std::string loggerName;
    LogLevel level{LogLevel::DEBUG};
    std::string data;
    LogSource source;
    TimestampMs timestamp{0};
    ThreadId threadId{0};
    ProcessId procId{0};

    LogMsg() = default;
    LogMsg(LogSource source, std::string_view loggerName, LogLevel level, std::string_view message);
};

}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_INTERNAL_LOG_MSG_H
