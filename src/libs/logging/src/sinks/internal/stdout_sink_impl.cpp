#include "sinks/internal/stdout_sink_impl.hpp"

#include <cstdio>
#include <string>

namespace origin::logging {
StdoutSinkImpl::StdoutSinkImpl() : SinkImplBase("StdoutSinkImpl.") {}

void StdoutSinkImpl::log_it(const LogMsg &logMsg)
{
    std::string content;
    _formatter->format(logMsg, content);
    sink_it(content);
}

void StdoutSinkImpl::sink_it(std::string_view message) const
{
    fprintf(m_stream, "%s\n", message.data());
    std::fflush(m_stream);
}

void StdoutSinkImpl::flush_it()
{
    std::fflush(m_stream);
}

}  // namespace origin::logging
