#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "detail/common.hpp"
#include "detail/mock_sinks/log_content_buffer_sink.hpp"
#include "gtest/gtest.h"
#include "logging/c/logging_c.h"
#include "logging/log_level.hpp"

using namespace origin::logging;

namespace logging_test {

class TestLoggerStPublicApi : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;

    std::string_view _loggerName = "TestLoggerStPublicApi";
    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>();
    SinkSt *_sinkSt = create_mock_sink_st(_sink);
    LoggerSt *_nullLogger = create_mock_logger_st(nullptr);
    FormatterSt *_nullFormatter = create_mock_formatter_st(nullptr);
    SinkSt const *_sinks[1] = {_sinkSt};
    LoggerSt *_loggerSt = origin_create_sync_logger(_loggerName.data(), _sinks, 1);
};

void TestLoggerStPublicApi::SetUp()
{
    ASSERT_NE(_loggerSt, nullptr);
    _sink->set_level(LogLevel::TRACE);
    _sink->enable_debug_info(false);
}

void TestLoggerStPublicApi::TearDown()
{
    destroy_mock_logger_st(_nullLogger);
    destroy_mock_sink_st(_sinkSt);
    destroy_mock_formatter_st(_nullFormatter);
    origin_destroy_logger(_loggerSt);
}

TEST_F(TestLoggerStPublicApi, get_name)
{
    EXPECT_STREQ(origin_logger_name(_loggerSt), _loggerName.data());
}

TEST_F(TestLoggerStPublicApi, get_name_failed_when_logger_nullptr)
{
    EXPECT_STREQ(origin_logger_name(nullptr), "");
    EXPECT_STREQ(origin_logger_name(_nullLogger), "");
}

TEST_F(TestLoggerStPublicApi, set_level)
{
    for (const LogLevelC level : C_LOG_LEVELS) {
        origin_logger_set_level(_loggerSt, level);
        EXPECT_EQ(origin_logger_level(_loggerSt), level);
        if (level != ORIGIN_LOG_LEVEL_OFF) {
            EXPECT_TRUE(origin_logger_should_log(_loggerSt, level));
        } else {
            EXPECT_FALSE(origin_logger_should_log(_loggerSt, level));
        }
    }
}

TEST_F(TestLoggerStPublicApi, set_level_failed_when_level_invalid)
{
    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);
    origin_logger_set_level(_loggerSt, INVALID_LEVEL_C);
    EXPECT_EQ(origin_logger_level(_loggerSt), ORIGIN_LOG_LEVEL_TRACE);
}

TEST_F(TestLoggerStPublicApi, set_level_failed_when_logger_nullptr)
{
    origin_logger_set_level(nullptr, ORIGIN_LOG_LEVEL_TRACE);
    origin_logger_set_level(_nullLogger, ORIGIN_LOG_LEVEL_TRACE);
}

TEST_F(TestLoggerStPublicApi, get_level_failed_when_logger_nullptr)
{
    EXPECT_EQ(origin_logger_level(nullptr), ORIGIN_LOG_LEVEL_OFF);
    EXPECT_EQ(origin_logger_level(_nullLogger), ORIGIN_LOG_LEVEL_OFF);
}

TEST_F(TestLoggerStPublicApi, should_log_false_when_logger_nullptr)
{
    EXPECT_FALSE(origin_logger_should_log(nullptr, ORIGIN_LOG_LEVEL_ERROR));
    EXPECT_FALSE(origin_logger_should_log(_nullLogger, ORIGIN_LOG_LEVEL_ERROR));
}

TEST_F(TestLoggerStPublicApi, should_log_false_when_level_invalid)
{
    EXPECT_FALSE(origin_logger_should_log(_loggerSt, INVALID_LEVEL_C));
    EXPECT_FALSE(origin_logger_should_log(_loggerSt, INVALID_LEVEL_C));
}

TEST_F(TestLoggerStPublicApi, flush_on)
{
    for (const LogLevelC level : C_LOG_LEVELS) {
        origin_logger_flush_on(_loggerSt, level);
        EXPECT_EQ(origin_logger_flush_level(_loggerSt), level);
        if (level != ORIGIN_LOG_LEVEL_OFF) {
            EXPECT_TRUE(origin_logger_should_flush(_loggerSt, level));
        } else {
            EXPECT_FALSE(origin_logger_should_flush(_loggerSt, level));
        }
    }
}

TEST_F(TestLoggerStPublicApi, flush_on_failed_when_level_invalid)
{
    origin_logger_flush_on(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);
    EXPECT_EQ(origin_logger_flush_level(_loggerSt), ORIGIN_LOG_LEVEL_TRACE);

    origin_logger_flush_on(_loggerSt, INVALID_LEVEL_C);
    EXPECT_EQ(origin_logger_flush_level(_loggerSt), ORIGIN_LOG_LEVEL_TRACE);
}

TEST_F(TestLoggerStPublicApi, flush_on_failed_when_logger_nullptr)
{
    origin_logger_flush_on(nullptr, ORIGIN_LOG_LEVEL_TRACE);
    origin_logger_flush_on(_nullLogger, ORIGIN_LOG_LEVEL_TRACE);
}

TEST_F(TestLoggerStPublicApi, get_flush_level_failed_when_logger_nullptr)
{
    EXPECT_EQ(origin_logger_flush_level(nullptr), ORIGIN_LOG_LEVEL_OFF);
    EXPECT_EQ(origin_logger_flush_level(_nullLogger), ORIGIN_LOG_LEVEL_OFF);
}

TEST_F(TestLoggerStPublicApi, should_flush_false_when_logger_nullptr)
{
    EXPECT_FALSE(origin_logger_should_flush(nullptr, ORIGIN_LOG_LEVEL_ERROR));
    EXPECT_FALSE(origin_logger_should_flush(_nullLogger, ORIGIN_LOG_LEVEL_ERROR));
}

TEST_F(TestLoggerStPublicApi, should_flush_false_when_level_invalid)
{
    EXPECT_FALSE(origin_logger_should_flush(_loggerSt, INVALID_LEVEL_C));
    EXPECT_FALSE(origin_logger_should_flush(_loggerSt, INVALID_LEVEL_C));
}

TEST_F(TestLoggerStPublicApi, set_pattern)
{
    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);

    origin_logger_set_pattern(_loggerSt, "%v");
    for (uint32_t i = 0; i < 10; i++) {
        ORIGIN_LOGGER_ERROR(_loggerSt, "%u", i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestLoggerStPublicApi, set_pattern_failed_when_logger_nullptr)
{
    origin_logger_set_pattern(nullptr, "%v");
    origin_logger_set_pattern(_nullLogger, "%v");
}

TEST_F(TestLoggerStPublicApi, set_pattern_failed_when_pattern_nullptr)
{
    origin_logger_set_pattern(_loggerSt, nullptr);
    ORIGIN_LOGGER_ERROR(_loggerSt, "test");
    EXPECT_NE("test", _sink->buffer()[0]);
}

TEST_F(TestLoggerStPublicApi, set_pattern_failed_when_pattern_empty)
{
    origin_logger_set_pattern(_loggerSt, "");
    ORIGIN_LOGGER_ERROR(_loggerSt, "test");
    EXPECT_NE("test", _sink->buffer()[0]);
}

TEST_F(TestLoggerStPublicApi, set_formatter)
{
    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);

    FormatterSt *formatter = origin_create_pattern_formatter("%v");
    origin_logger_set_formatter(_loggerSt, formatter);
    origin_destroy_formatter(formatter);

    for (uint32_t i = 0; i < 10; i++) {
        ORIGIN_LOGGER_ERROR(_loggerSt, "%u", i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestLoggerStPublicApi, set_formatter_failed_when_logger_nullptr)
{
    FormatterSt *formatter = origin_create_pattern_formatter("%v");
    origin_logger_set_formatter(nullptr, formatter);
    origin_logger_set_formatter(_nullLogger, formatter);
    origin_destroy_formatter(formatter);
}

TEST_F(TestLoggerStPublicApi, set_formatter_failed_when_formatter_nullptr)
{
    origin_logger_set_formatter(_loggerSt, nullptr);
    origin_logger_set_formatter(_loggerSt, _nullFormatter);
}

TEST_F(TestLoggerStPublicApi, log_failed_when_logger_nullptr)
{
    ORIGIN_LOGGER_LOG(nullptr, ORIGIN_LOG_LEVEL_ERROR, "Test LoggerSt public api.");
    ORIGIN_LOGGER_LOG(_nullLogger, ORIGIN_LOG_LEVEL_ERROR, "Test LoggerSt public api.");
}

TEST_F(TestLoggerStPublicApi, log_failed_when_level_invlid)
{
    ORIGIN_LOGGER_LOG(_loggerSt, INVALID_LEVEL_C, "Test LoggerSt public api.");
    EXPECT_EQ(_sink->buffer().size(), 0);
}

TEST_F(TestLoggerStPublicApi, log_failed_when_formtat_nullptr)
{
    ORIGIN_LOGGER_ERROR(_loggerSt, nullptr);
    EXPECT_EQ(_sink->buffer().size(), 0);
}

TEST_F(TestLoggerStPublicApi, flush_failed_when_logger_nullptr)
{
    ORIGIN_LOGGER_ERROR(_loggerSt, "Test LoggerSt public api.");
    EXPECT_EQ(_sink->buffer().size(), 1);
    origin_logger_flush(nullptr);
    origin_logger_flush(_nullLogger);
    EXPECT_EQ(_sink->buffer().size(), 1);
    EXPECT_EQ(_sink->disk().size(), 0);
}

}  // namespace logging_test
