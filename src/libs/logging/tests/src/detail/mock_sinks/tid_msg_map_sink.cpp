#include "detail/mock_sinks/tid_msg_map_sink.hpp"

#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "common/debug/debug_logger.h"
#include "common/types/thread_types.h"
#include "logging/log_msg.hpp"

using namespace origin::logging;

namespace logging_test {
void TidMsgMapSink::log_it(const LogMsg& logMsg)
{
    if (_enableDebugInfo) {
        std::string content;
        _formatter->format(logMsg, content);
        std::cout << content << std::endl;
    }
    _buffer[logMsg.threadId].push_back(logMsg);
}

const std::map<ThreadId, std::vector<LogMsg>>& TidMsgMapSink::buffer()
{
    std::lock_guard<std::mutex> const lock(_sinkMtx);
    return _buffer;
}

void TidMsgMapSink::clear_it()
{
    if (_enableDebugInfo) {
        ORIGIN_DEBUG_INFO("TidMsgMapSink clear. buffer: {}.", _buffer.size());
    }
    _buffer.clear();
}
}  // namespace logging_test
