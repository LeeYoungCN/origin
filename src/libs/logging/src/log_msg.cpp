#include "logging/log_msg.h"

#include <string>
#include <string_view>

#include "logging/log_level.h"
#include "logging/log_source.h"
#include "utils/date_time_utils.h"
#include "utils/process_utils.h"
#include "utils/thread_utils.h"

namespace origin::logging {
LogMsg create_log_msg(const LogSource& source, std::string_view loggerName, LogLevel level,
                      std::string_view message)
{
    LogMsg msg{};
    init_log_msg(msg, source, loggerName, level, message);
    return msg;
}

void init_log_msg(LogMsg& msg, const LogSource& source, std::string_view loggerName, LogLevel level,
                  std::string_view message)
{
    msg.loggerName = loggerName;
    msg.level = level;
    msg.data = message;
    msg.source = source;
    msg.timestamp = origin::date_time::get_now_timestamp_ms();
    msg.threadId = origin::thread::get_curr_thread_id();
    msg.procId = origin::process::get_curr_proc_id();
}
}  // namespace origin::logging
