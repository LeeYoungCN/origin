#include "tid_msg_map_sink.h"

#include <mutex>

using namespace origin::logging;

namespace logging_test {
void TidMsgMapSink::log_it(const LogMsg& logMsg)
{
    _buffer[logMsg.threadId] = logMsg;
}

const std::map<ThreadId, LogMsg>& TidMsgMapSink::buffer()
{
    std::lock_guard<std::mutex> lock(_sinkMtx);
    return _buffer;
}
}  // namespace logging_test
