#ifndef TEST_LOGGING_TEST_UTILS_COMMON_H
#define TEST_LOGGING_TEST_UTILS_COMMON_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "logging/c/logging_c.h"
#include "logging/log_level.hpp"
#include "logging/sinks/sink.hpp"

struct MockSinkSt {
    std::shared_ptr<origin::logging::Sink> ptr;
    MockSinkSt() = default;
    explicit MockSinkSt(std::shared_ptr<origin::logging::Sink> ptr) : ptr(std::move(ptr)) {}
};

namespace logging_test {
const std::vector<origin::logging::LogLevel> LOG_LEVELS = {origin::logging::LogLevel::TRACE,
                                                           origin::logging::LogLevel::DEBUG,
                                                           origin::logging::LogLevel::INFO,
                                                           origin::logging::LogLevel::WARN,
                                                           origin::logging::LogLevel::ERR,
                                                           origin::logging::LogLevel::FATAL,
                                                           origin::logging::LogLevel::OFF};

const std::vector<OriginLogLevel> C_LOG_LEVELS = {ORIGIN_LOG_LEVEL_TRACE,
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
}  // namespace logging_test
#endif  // TEST_LOGGING_TEST_UTILS_COMMON_H
