#ifndef ORIGIN_LOGGING_SINKS_STDERR_SINK_HPP
#define ORIGIN_LOGGING_SINKS_STDERR_SINK_HPP

#include "logging/detail/logging_api.h"
#include "logging/sinks/sink_base.hpp"

namespace origin::logging {
class LOGGING_API StderrSink : public SinkBase {
public:
    StderrSink();
    ~StderrSink() override = default;
};
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_SINKS_STDERR_SINK_HPP
