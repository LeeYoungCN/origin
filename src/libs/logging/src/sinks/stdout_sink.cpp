#include "logging/sinks/stdout_sink.h"

#include <memory>

#include "logging/sinks/sink_base.h"
#include "sinks/internal/stdout_sink_impl.h"

namespace origin::logging {
StdoutSink::StdoutSink() : SinkBase(std::make_unique<StdoutSinkImpl>()) {}

StdoutSink::StdoutSink(FILE *file) : SinkBase(std::make_unique<StdoutSinkImpl>(file)) {}

}  // namespace origin::logging
