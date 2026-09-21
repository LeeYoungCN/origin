#include "sinks/internal/stream_sink_impl.hpp"

#include <iostream>
#include <string>
#include <string_view>

#include "logging/log_msg.hpp"
#include "sinks/internal/sink_impl_base.hpp"

namespace origin::logging {
StreamSinkImpl::StreamSinkImpl() : SinkImplBase("StreamSink.") {}

StreamSinkImpl::StreamSinkImpl(std::ostream &stream, const std::string_view param_string)
    : SinkImplBase(param_string), _stream(stream)
{
}

void StreamSinkImpl::log_it(const LogMsg &logMsg)
{
    std::string content;
    _formatter->format(logMsg, content);
    sink_it(content);
}

void StreamSinkImpl::sink_it(const std::string_view message) const
{
    _stream << message << '\n';
}

void StreamSinkImpl::flush_it()
{
    _stream.flush();
}

}  // namespace origin::logging
