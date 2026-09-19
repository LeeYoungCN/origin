#ifndef LOGGING_TEST_BASE_TEST_SINK_H
#define LOGGING_TEST_BASE_TEST_SINK_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "logging/formatters/formatter.hpp"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/log_msg.hpp"
#include "logging/sinks/sink.hpp"

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
    [[nodiscard]] std::string_view param_string() const override;

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

#endif  // LOGGING_TEST_BASE_TEST_SINK_H
