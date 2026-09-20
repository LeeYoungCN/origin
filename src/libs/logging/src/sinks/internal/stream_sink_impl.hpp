#ifndef ORIGIN_LOGGING_SINKS_INTERNAL_STREAM_SINK_IMPL_HPP
#define ORIGIN_LOGGING_SINKS_INTERNAL_STREAM_SINK_IMPL_HPP

#include <iostream>
#include <ostream>
#include <string_view>

#include "logging/log_msg.hpp"
#include "sinks/internal/sink_impl_base.hpp"

namespace origin::logging {
class StreamSinkImpl : public SinkImplBase {
public:
    StreamSinkImpl();
    StreamSinkImpl(std::ostream& stream, std::string_view param_string);
    ~StreamSinkImpl() override = default;

protected:
    void log_it(const LogMsg& logMsg) override;
    void flush_it() override;
    void sink_it(std::string_view message) const;

private:
    std::ostream& _stream = std::cout;
};
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_SINKS_INTERNAL_STREAM_SINK_IMPL_HPP
