#pragma once
#ifndef ORIGIN_LOGGING_SINKS_STDOUT_SINK_HPP
#define ORIGIN_LOGGING_SINKS_STDOUT_SINK_HPP

#include "logging/logging_api.h"
#include "logging/sinks/sink_base.hpp"

namespace origin::logging {
class LOGGING_API StdoutSink : public SinkBase {
public:
    StdoutSink();
    ~StdoutSink() override = default;
};
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_SINKS_STDOUT_SINK_HPP
