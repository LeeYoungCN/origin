#include "logging/sinks/stderr_sink.hpp"

#include <iostream>
#include <memory>

#include "logging/sinks/sink_base.hpp"
#include "sinks/internal/stream_sink_impl.hpp"

namespace origin::logging {
StderrSink::StderrSink() : SinkBase(std::make_unique<StreamSinkImpl>(std::cerr, "StderrSink.")) {}

}  // namespace origin::logging
