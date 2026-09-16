#ifndef TEST_LOGGING_DETAIL_COMMON_H
#define TEST_LOGGING_DETAIL_COMMON_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "logging/c/logging_c.h"
#include "logging/formatters/formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/loggers/logger.hpp"
#include "logging/sinks/sink.hpp"

constexpr auto INVALID_LEVEL_C = static_cast<LogLevelC>(ORIGIN_LOG_LEVEL_OFF + 1);

struct MockLoggerSt {
    std::shared_ptr<origin::logging::Logger> ptr;
    MockLoggerSt() = default;
    explicit MockLoggerSt(std::shared_ptr<origin::logging::Logger> ptr) : ptr(std::move(ptr)) {}
};

struct MockSinkSt {
    std::shared_ptr<origin::logging::Sink> ptr;
    MockSinkSt() = default;
    explicit MockSinkSt(std::shared_ptr<origin::logging::Sink> ptr) : ptr(std::move(ptr)) {}
};

struct MockFormatterSt {
    std::unique_ptr<origin::logging::Formatter> ptr;

    MockFormatterSt() = default;
    explicit MockFormatterSt(std::unique_ptr<origin::logging::Formatter> formatter)
        : ptr(std::move(formatter))
    {
    }
    explicit MockFormatterSt(origin::logging::Formatter *formatter) : ptr(formatter) {}
};

namespace logging_test {
const std::vector<origin::logging::LogLevel> LOG_LEVELS = {origin::logging::LogLevel::TRACE,
                                                           origin::logging::LogLevel::DEBUG,
                                                           origin::logging::LogLevel::INFO,
                                                           origin::logging::LogLevel::WARN,
                                                           origin::logging::LogLevel::ERR,
                                                           origin::logging::LogLevel::FATAL,
                                                           origin::logging::LogLevel::OFF};

const std::vector<LogLevelC> C_LOG_LEVELS = {ORIGIN_LOG_LEVEL_TRACE,
                                             ORIGIN_LOG_LEVEL_DEBUG,
                                             ORIGIN_LOG_LEVEL_INFO,
                                             ORIGIN_LOG_LEVEL_WARN,
                                             ORIGIN_LOG_LEVEL_ERROR,
                                             ORIGIN_LOG_LEVEL_FATAL,
                                             ORIGIN_LOG_LEVEL_OFF};

int32_t diff_log_level(origin::logging::LogLevel a, origin::logging::LogLevel b);
std::string get_logger_name(const testing::TestInfo *test_info);

std::string get_log_dir();

SinkSt *create_mock_sink_st(std::shared_ptr<origin::logging::Sink> sink);
void destroy_mock_sink_st(SinkSt *sink);

LoggerSt *create_mock_logger_st(std::shared_ptr<origin::logging::Logger> logger);
void destroy_mock_logger_st(LoggerSt *logger);

FormatterSt *create_mock_formatter_st(std::unique_ptr<origin::logging::Formatter> formatter);
void destroy_mock_formatter_st(FormatterSt *formatter);

}  // namespace logging_test
#endif  // TEST_LOGGING_DETAIL_COMMON_H
