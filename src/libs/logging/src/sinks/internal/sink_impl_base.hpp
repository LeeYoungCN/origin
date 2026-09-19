#pragma once
#ifndef ORIGIN_LOGGING_SINKS_INTERNAL_BASE_SINK_IMPL_HPP
#define ORIGIN_LOGGING_SINKS_INTERNAL_BASE_SINK_IMPL_HPP
#include <atomic>
#include <memory>
#include <mutex>
#include <string_view>

#include "logging/formatters/formatter.hpp"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/log_msg.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {
class SinkImplBase : public Sink {
public:
    SinkImplBase() = default;
    ~SinkImplBase() override = default;
    explicit SinkImplBase(std::string_view paramString);

    void log(const LogMsg& logMsg) override;
    void flush() override;

    void set_pattern(std::string_view pattern) override;
    void set_formatter(std::unique_ptr<Formatter> formatter) override;

    [[nodiscard]] bool should_log(LogLevel level) const override;
    void set_level(LogLevel level) override;
    [[nodiscard]] LogLevel level() const override;

    [[nodiscard]] std::string_view param_string() const override;

protected:
    virtual void log_it(const LogMsg& logMsg) = 0;
    virtual void flush_it() = 0;
    void set_param_string(std::string_view param);

    std::atomic<LogLevel> _level{LogLevel::INFO};
    std::unique_ptr<Formatter> _formatter{std::make_unique<PatternFormatter>()};
    mutable std::mutex _sinkMtx;
    std::string _paramStr{"unknown"};
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_SINKS_INTERNAL_BASE_SINK_IMPL_HPP
