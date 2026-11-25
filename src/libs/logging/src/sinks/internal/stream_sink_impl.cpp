#include "sinks/internal/stream_sink_impl.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "origin/logging/log_msg.hpp"

#include "sinks/internal/sink_impl_base.hpp"

namespace origin::logging {
StreamSinkImpl::StreamSinkImpl() : SinkImplBase("StreamSink.") {}

StreamSinkImpl::StreamSinkImpl(std::ostream& stream, const std::string_view param_string)
    : SinkImplBase(param_string), _stream(stream)
{
}

void StreamSinkImpl::log_it(const LogMsg& logMsg)
{
    std::string content;
    _formatter->format(logMsg, content);
    sink_it(content);
}

void StreamSinkImpl::sink_it(const std::string_view message) const
{
    _stream << message << '\n';
    if (_stream.fail()) {
        throw std::runtime_error("Failed to write stream log.");
    }
}

void StreamSinkImpl::flush_it()
{
    _stream.flush();
    if (_stream.fail()) {
        throw std::runtime_error("Failed to flush stream log.");
    }
}

}  // namespace origin::logging
