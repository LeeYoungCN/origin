#include "origin/logging/log_msg.hpp"

#include <string>
#include <string_view>

#include "origin/logging/log_level.hpp"
#include "origin/logging/log_source.hpp"
#include "origin/utils/date_time_utils.h"
#include "origin/utils/process_utils.h"
#include "origin/utils/thread_utils.h"

namespace origin::logging {
LogMsg create_log_msg(const LogSource& source, const std::string_view loggerName,
                      const LogLevel level, const std::string_view message)
{
    LogMsg msg{};
    init_log_msg(source, loggerName, level, message, msg);
    return msg;
}

void init_log_msg(const LogSource& source, const std::string_view loggerName, const LogLevel level,
                  const std::string_view message, LogMsg& msg)
{
    msg.loggerName = loggerName;
    msg.level = level;
    msg.data = message;
    msg.source = source;
    msg.timestamp = date_time::get_now_timestamp_ms();
    msg.threadId = thread::get_curr_thread_id();
    msg.procId = process::get_curr_proc_id();
}
}  // namespace origin::logging
