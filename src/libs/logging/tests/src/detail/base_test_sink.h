#pragma once
#ifndef TEST_LOGGING_TEST_UTILS_BASE_TEST_SINK_H
#define TEST_LOGGING_TEST_UTILS_BASE_TEST_SINK_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string_view>

#include "internal/log_msg.h"
#include "logging/formatters/formatter.h"
#include "logging/formatters/pattern_formatter.h"
#include "logging/log_level.h"
#include "logging/sinks/sink.h"

using namespace origin::logging;

namespace logging_test {
class BaseTestSink : public Sink {
public:
    BaseTestSink() = default;
    ~BaseTestSink() override;

    void log(const LogMsg& logMsg) override;
    void flush() override;

    void set_pattern(std::string_view pattern) override;
    void set_formatter(std::unique_ptr<Formatter> formatter) override;

    [[nodiscard]] bool should_log(LogLevel level) const override;
    void set_level(LogLevel level) override;
    [[nodiscard]] LogLevel level() const override;
    [[nodiscard]] std::string_view param_str() const override;

protected:
    explicit BaseTestSink(std::string_view parameter);
    virtual void log_it(const LogMsg& logMsg) = 0;
    virtual void flush_it() = 0;

protected:
    std::atomic<LogLevel> _level{LogLevel::INFO};
    std::unique_ptr<Formatter> _formatter{std::make_unique<PatternFormatter>()};
    std::mutex _sinkMtx;
    std::string _paramStr{"unknown"};
};
}  // namespace logging_test

#endif  // TEST_LOGGING_TEST_UTILS_BASE_TEST_SINK_H
