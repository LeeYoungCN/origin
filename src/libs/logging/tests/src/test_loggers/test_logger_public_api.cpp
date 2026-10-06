#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "detail/common.hpp"
#include "detail/mock_sinks/log_content_buffer_sink.hpp"
#include "gtest/gtest.h"
#include "logging/formatters/formatter.hpp"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/loggers/sync_logger.hpp"

using namespace logging_test;
using namespace origin::logging;

namespace logging_test {

class TestLoggerPublicApi : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};

    void init_logger(const testing::TestInfo* test_info);
    std::shared_ptr<SyncLogger> _logger;
    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>();
};

void TestLoggerPublicApi::init_logger(const testing::TestInfo* test_info)
{
    const std::string name = get_logger_name(test_info);
    _logger = std::make_shared<SyncLogger>(name, _sink);
}

TEST_F(TestLoggerPublicApi, log_level)
{
    init_logger(test_info_);
    for (const LogLevel fileterLevel : LOG_LEVELS) {
        _logger->set_level(fileterLevel);
        EXPECT_EQ(_logger->level(), fileterLevel);
        for (const LogLevel level : LOG_LEVELS) {
            if (fileterLevel != LogLevel::OFF) {
                EXPECT_TRUE(_logger->should_log(fileterLevel));
            } else {
                if (level >= fileterLevel && level != LogLevel::OFF) {
                    EXPECT_TRUE(_logger->should_log(fileterLevel));
                } else {
                    EXPECT_FALSE(_logger->should_log(fileterLevel));
                }
            }
        }
    }
}

TEST_F(TestLoggerPublicApi, flush_level)
{
    init_logger(test_info_);
    for (const LogLevel flushLevel : LOG_LEVELS) {
        _logger->flush_on(flushLevel);
        EXPECT_EQ(_logger->flush_level(), flushLevel);
        for (const LogLevel level : LOG_LEVELS) {
            if (flushLevel != LogLevel::OFF) {
                EXPECT_TRUE(_logger->should_flush(flushLevel));
            } else {
                if (level >= flushLevel && level != LogLevel::OFF) {
                    EXPECT_TRUE(_logger->should_flush(flushLevel));
                } else {
                    EXPECT_FALSE(_logger->should_flush(flushLevel));
                }
            }
        }
    }
}

TEST_F(TestLoggerPublicApi, set_pattern)
{
    init_logger(test_info_);
    _logger->set_pattern("%v");
    for (uint32_t i = 0; i < 100; i++) {
        _logger->error(i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestLoggerPublicApi, set_pattern_failed_when_empty)
{
    init_logger(test_info_);
    _logger->set_pattern("%v");
    _logger->error("test");
    EXPECT_EQ("test", _sink->buffer()[0]);
    EXPECT_THROW(_logger->set_pattern(""), std::invalid_argument);
    _logger->error("test");
    EXPECT_EQ("test", _sink->buffer()[1]);
}

TEST_F(TestLoggerPublicApi, set_formatter)
{
    init_logger(test_info_);
    _sink->set_level(LogLevel::TRACE);
    _logger->set_level(LogLevel::TRACE);
    const std::unique_ptr<Formatter> formatter = std::make_unique<PatternFormatter>("%v");
    _logger->set_formatter(formatter);
    for (uint32_t i = 0; i < 100; i++) {
        _logger->error(i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestLoggerPublicApi, set_formatter_failed_when_nullptr)
{
    init_logger(test_info_);
    _logger->set_pattern("%v");
    _logger->error("test");
    EXPECT_EQ("test", _sink->buffer()[0]);
    _logger->set_formatter(nullptr);
    _logger->error("test");
    EXPECT_EQ("test", _sink->buffer()[1]);
}

}  // namespace logging_test
