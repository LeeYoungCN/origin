#include "internal/log_msg.h"

#include <string>
#include <string_view>
#include <utility>

#include "logging/log_level.h"
#include "logging/log_source.h"
#include "utils/date_time_utils.h"
#include "utils/process_utils.h"
#include "utils/thread_utils.h"

namespace origin::logging {
LogMsg::LogMsg(LogSource source, std::string_view loggerName, LogLevel level,
               std::string_view message)
    : loggerName(loggerName),
      level(level),
      data(message),
      source(std::move(source)),
      timestamp(origin::date_time::get_now_timestamp_ms()),
      threadId(origin::thread::get_curr_thread_id()),
      procId(origin::process::get_curr_proc_id())
{
}
}  // namespace origin::logging
