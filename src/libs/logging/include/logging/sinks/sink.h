#pragma once
#ifndef ORIGIN_LOGGING_SINKS_SINK_H
#define ORIGIN_LOGGING_SINKS_SINK_H

#include <memory>
#include <string_view>

#include "logging/formatters/formatter.h"
#include "logging/log_level.h"
#include "logging/logging_api.h"

namespace origin::logging {
struct LogMsg;
class LOGGING_API Sink {
public:
    Sink() = default;
    virtual ~Sink() = default;

    virtual void log(const LogMsg& logMsg) = 0;
    virtual void flush() = 0;

    virtual void set_pattern(std::string_view pattern) = 0;
    virtual void set_formatter(std::unique_ptr<Formatter> formatter) = 0;

    [[nodiscard]] virtual bool should_log(LogLevel level) const = 0;
    virtual void set_level(LogLevel level) = 0;
    [[nodiscard]] virtual LogLevel level() const = 0;
    [[nodiscard]] virtual std::string_view param_str() const = 0;
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_SINKS_SINK_H
