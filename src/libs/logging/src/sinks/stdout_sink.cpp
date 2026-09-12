#include "logging/sinks/stdout_sink.hpp"

#include <memory>

#include "logging/sinks/sink_base.hpp"
#include "sinks/internal/stdout_sink_impl.hpp"

namespace origin::logging {
StdoutSink::StdoutSink() : SinkBase(std::make_unique<StdoutSinkImpl>()) {}

}  // namespace origin::logging
