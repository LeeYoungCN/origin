#include "sinks/internal/stream_sink_impl.hpp"

#include <cstdio>
#include <string>
#include <string_view>
#include "logging/log_msg.hpp"
#include "sinks/internal/sink_impl_base.hpp"

namespace origin::logging {
StreamSinkImpl::StreamSinkImpl() : SinkImplBase("StdoutSink.") {}

StreamSinkImpl::StreamSinkImpl(const StreamType type)
{
    if (type == StreamType::STDOUT) {
        _stream = stdout;
        set_param_string("StdoutSink.");
    } else {
        _stream = stderr;
        set_param_string("StderrSink.");
    }
}

void StreamSinkImpl::log_it(const LogMsg &logMsg)
{
    std::string content;
    _formatter->format(logMsg, content);
    sink_it(content);
}

void StreamSinkImpl::sink_it(const std::string_view message) const
{
    std::fprintf(_stream, "%s\n", message.data());
    std::fflush(_stream);
}

void StreamSinkImpl::flush_it()
{
    std::fflush(_stream);
}

}  // namespace origin::logging
