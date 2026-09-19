#include "logging/sinks/stderr_sink.hpp"

#include <memory>

#include "logging/sinks/sink_base.hpp"
#include "sinks/internal/stream_sink_impl.hpp"

namespace origin::logging {
StderrSink::StderrSink() : SinkBase(std::make_unique<StreamSinkImpl>(StreamType::STDERR)) {}

}  // namespace origin::logging
