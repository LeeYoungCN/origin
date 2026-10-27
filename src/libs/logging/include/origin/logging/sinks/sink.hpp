#ifndef ORIGIN_LOGGING_SINKS_SINK_HPP
#define ORIGIN_LOGGING_SINKS_SINK_HPP

#include <memory>
#include <string_view>

#include "origin/logging/detail/logging_api.h"
#include "origin/logging/formatters/formatter.hpp"
#include "origin/logging/log_level.hpp"

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
    [[nodiscard]] virtual std::string_view param_string() const = 0;
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_SINKS_SINK_HPP
