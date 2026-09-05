#ifndef TEST_LOGGING_TEST_UTILS_COMMON_H
#define TEST_LOGGING_TEST_UTILS_COMMON_H

#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "logging/log_level.h"

namespace logging_test {
const std::vector<origin::logging::LogLevel> LOG_LEVELS = {origin::logging::LogLevel::TRACE,
                                                           origin::logging::LogLevel::DEBUG,
                                                           origin::logging::LogLevel::INFO,
                                                           origin::logging::LogLevel::WARN,
                                                           origin::logging::LogLevel::ERR,
                                                           origin::logging::LogLevel::FATAL,
                                                           origin::logging::LogLevel::OFF};

std::string get_logger_name(const testing::TestInfo* test_info);

std::string get_log_dir();

}  // namespace logging_test
#endif  // TEST_LOGGING_TEST_UTILS_COMMON_H
