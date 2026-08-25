#include "sinks/internal/stdout_sink_impl.h"

#include <cstdio>
#include <string>

namespace origin::logging {
StdoutSinkImpl::StdoutSinkImpl() : StdoutSinkImpl(stdout) {}

StdoutSinkImpl::StdoutSinkImpl(FILE *file) : m_stream(file == nullptr ? stdout : file)
{
    if (m_stream == stdout) {
        _paramStr = std::string("StdoutSinkImpl, stream: stdout.");
    } else if (m_stream == stderr) {
        _paramStr = std::string("StdoutSinkImpl, stream: stderr.");
    } else {
        _paramStr = std::string("StdoutSinkImpl, stream: " +
                                std::to_string(reinterpret_cast<std::uintptr_t>(m_stream)) + ".");
    }
}

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
