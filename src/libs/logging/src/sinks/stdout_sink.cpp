#include "logging/sinks/stdout_sink.hpp"

#include <iostream>
#include <memory>

#include "logging/sinks/sink_base.hpp"
#include "sinks/internal/stream_sink_impl.hpp"

namespace origin::logging {
StdoutSink::StdoutSink() : SinkBase(std::make_unique<StreamSinkImpl>(std::cout, "StdoutSink.")) {}

}  // namespace origin::logging
