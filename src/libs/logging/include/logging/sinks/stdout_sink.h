#pragma once
#ifndef ORIGIN_LOGGING_SINKS_STDOUT_SINK_H
#define ORIGIN_LOGGING_SINKS_STDOUT_SINK_H

#include <cstdio>

#include "logging/logging_api.h"
#include "logging/sinks/sink_base.h"

namespace origin::logging {
class LOGGING_API StdoutSink : public SinkBase {
public:
    StdoutSink();
    ~StdoutSink() override = default;
    explicit StdoutSink(FILE *file);
};
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_SINKS_STDOUT_SINK_H
