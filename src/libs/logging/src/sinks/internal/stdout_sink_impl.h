#pragma once
#ifndef ORIGIN_LOGGING_SINKS_INTERNAL_STDOUT_SINK_IMPL_H
#define ORIGIN_LOGGING_SINKS_INTERNAL_STDOUT_SINK_IMPL_H

#include <cstdio>
#include <string_view>

#include "sinks/internal/sink_impl_base.h"

namespace origin::logging {
class StdoutSinkImpl : public SinkImplBase {
public:
    StdoutSinkImpl();
    ~StdoutSinkImpl() override = default;
    explicit StdoutSinkImpl(FILE *file);

private:
    void log_it(const LogMsg &logMsg) override;
    void flush_it() override;
    void sink_it(std::string_view message) const;

private:
    FILE *m_stream = stdout;
};
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_SINKS_INTERNAL_STDOUT_SINK_IMPL_H
