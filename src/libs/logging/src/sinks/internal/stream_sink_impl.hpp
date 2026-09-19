#pragma once
#ifndef ORIGIN_LOGGING_SINKS_INTERNAL_STREAM_SINK_IMPL_HPP
#define ORIGIN_LOGGING_SINKS_INTERNAL_STREAM_SINK_IMPL_HPP

#include <cstdio>
#include <string_view>

#include "logging/log_msg.hpp"
#include "sinks/internal/sink_impl_base.hpp"

namespace origin::logging {
enum class StreamType {
    STDOUT,
    STDERR
};

class StreamSinkImpl : public SinkImplBase {
public:
    StreamSinkImpl();
    explicit StreamSinkImpl(StreamType type);
    ~StreamSinkImpl() override = default;

protected:
    void log_it(const LogMsg &logMsg) override;
    void flush_it() override;
    void sink_it(std::string_view message) const;

private:
    FILE *_stream = stdout;
};
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_SINKS_INTERNAL_STREAM_SINK_IMPL_HPP
