#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "detail/common.hpp"
#include "detail/mock_sinks/log_content_buffer_sink.hpp"
#include "gtest/gtest.h"
#include "logging/c/logging_c.h"
#include "logging/log_level.hpp"

using namespace origin::logging;

namespace logging_test {

class TestRootLoggerC : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;
    void init_root_logger(const testing::TestInfo *test_info);

    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>();
    LoggerSt *_loggerSt = nullptr;
    SinkSt *_sinkSt = create_mock_sink_st(_sink);
    SinkSt const *_sinks[1] = {_sinkSt};
    LoggerSt *_nullLogger = create_mock_logger_st(nullptr);
    FormatterSt *_nullFormatter = create_mock_formatter_st(nullptr);
};

void TestRootLoggerC::SetUp()
{
    _sink->set_level(LogLevel::TRACE);
    _sink->enable_debug_info(false);
    origin_shutdown();
}

void TestRootLoggerC::TearDown()
{
    destroy_mock_sink_st(_sinkSt);
    origin_destroy_logger(_loggerSt);
    destroy_mock_logger_st(_nullLogger);
    destroy_mock_formatter_st(_nullFormatter);
    origin_shutdown();
}

void TestRootLoggerC::init_root_logger(const testing::TestInfo *test_info)
{
    const std::string name = get_logger_name(test_info);
    _loggerSt = origin_create_sync_logger(name.data(), _sinks, 1);
    ASSERT_NE(_loggerSt, nullptr);
    origin_set_root_logger(_loggerSt);
    origin_set_level(ORIGIN_LOG_LEVEL_TRACE);
}

TEST_F(TestRootLoggerC, set_and_get_root_logger)
{
    const std::string name = get_logger_name(test_info_);
    _loggerSt = origin_create_sync_logger(name.data(), _sinks, 1);
    ASSERT_NE(_loggerSt, nullptr);
    origin_set_root_logger(_loggerSt);
    const auto rootLogger = origin_root_logger();
    ASSERT_NE(rootLogger, nullptr);
    EXPECT_EQ(origin_logger_name(rootLogger), name);
    origin_destroy_logger(rootLogger);
}

TEST_F(TestRootLoggerC, set_root_logger_failed_when_logger_nullptr)
{
    init_root_logger(test_info_);

    origin_set_root_logger(nullptr);
    const auto rootLogger = origin_root_logger();
    ASSERT_NE(rootLogger, nullptr);
    EXPECT_EQ(origin_logger_name(rootLogger), origin_logger_name(_loggerSt));
    origin_destroy_logger(rootLogger);
}

TEST_F(TestRootLoggerC, set_root_logger_failed_when_logger_ptr_nullptr)
{
    init_root_logger(test_info_);

    origin_set_root_logger(_nullLogger);
    const auto rootLogger = origin_root_logger();
    ASSERT_NE(rootLogger, nullptr);
    EXPECT_EQ(origin_logger_name(rootLogger), origin_logger_name(_loggerSt));
    origin_destroy_logger(rootLogger);
}

TEST_F(TestRootLoggerC, get_root_logger_failed_when_nonexist)
{
    EXPECT_EQ(origin_root_logger(), nullptr);
}

TEST_F(TestRootLoggerC, log_macros)
{
    init_root_logger(test_info_);

    constexpr uint32_t logCount = 100;
    for (uint32_t i = 0; i < logCount; ++i) {
        ORIGIN_LOGGING_TRACE(
            "Level: [%s], idx: %u", origin_log_level_full_name(ORIGIN_LOG_LEVEL_TRACE), i);
        ORIGIN_LOGGING_DEBUG(
            "Level: [%s], idx: %u", origin_log_level_full_name(ORIGIN_LOG_LEVEL_DEBUG), i);
        ORIGIN_LOGGING_INFO(
            "Level: [%s], idx: %u", origin_log_level_full_name(ORIGIN_LOG_LEVEL_INFO), i);
        ORIGIN_LOGGING_WARN(
            "Level: [%s], idx: %u", origin_log_level_full_name(ORIGIN_LOG_LEVEL_WARN), i);
        ORIGIN_LOGGING_ERROR(
            "Level: [%s], idx: %u", origin_log_level_full_name(ORIGIN_LOG_LEVEL_ERROR), i);
        ORIGIN_LOGGING_FATAL(
            "Level: [%s], idx: %u", origin_log_level_full_name(ORIGIN_LOG_LEVEL_FATAL), i);
    }

    _sink->flush();
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), logCount * (LOG_LEVELS.size() - 1));
}

TEST_F(TestRootLoggerC, log_flush)
{
    init_root_logger(test_info_);
    constexpr uint32_t MAX_ITEM_CNT = 100;
    for (uint32_t i = 0; i < MAX_ITEM_CNT; ++i) {
        ORIGIN_LOGGING_ERROR("test log: %u", i);
        EXPECT_EQ(_sink->buffer().size(), i + 1);
        EXPECT_EQ(_sink->disk().size(), 0);
    }
    origin_flush();
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), MAX_ITEM_CNT);
}

TEST_F(TestRootLoggerC, log_level_filter)
{
    init_root_logger(test_info_);

    for (const LogLevelC filterLevel : C_LOG_LEVELS) {
        origin_set_level(filterLevel);
        for (const LogLevelC logLevel : C_LOG_LEVELS) {
            ORIGIN_LOGGING_LOG(logLevel,
                               "FilterLevel: [%s], Level: [%s].",
                               origin_log_level_full_name(filterLevel),
                               origin_log_level_full_name(logLevel));
        }
        if (filterLevel != ORIGIN_LOG_LEVEL_OFF) {
            EXPECT_EQ(_sink->buffer().size(), ORIGIN_LOG_LEVEL_FATAL - origin_level() + 1);
        } else {
            EXPECT_EQ(_sink->buffer().size(), 0);
        }
        _sink->clear();
    }
}

TEST_F(TestRootLoggerC, flush_level_filter)
{
    init_root_logger(test_info_);

    for (LogLevelC const flushLevel : C_LOG_LEVELS) {
        // 设置刷新等级
        origin_flush_on(flushLevel);
        for (uint32_t i = 0; i < C_LOG_LEVELS.size(); ++i) {
            LogLevelC const level = C_LOG_LEVELS[i];
            if (level == ORIGIN_LOG_LEVEL_OFF) {
                break;
            }
            ORIGIN_LOGGING_LOG(level,
                               "FlushLevel: [%s], Level: [%s].",
                               origin_log_level_abbr_name(flushLevel),
                               origin_log_level_abbr_name(level));

            if (flushLevel == ORIGIN_LOG_LEVEL_OFF || level < flushLevel) {
                EXPECT_EQ(_sink->buffer().size(), i + 1);
                EXPECT_EQ(_sink->disk().size(), 0);
            } else {
                EXPECT_EQ(_sink->buffer().size(), 0);
                EXPECT_EQ(_sink->disk().size(), i + 1);
            }
        }
        _sink->clear();
    }
}

TEST_F(TestRootLoggerC, set_and_get_level)
{
    init_root_logger(test_info_);
    for (const LogLevelC level : C_LOG_LEVELS) {
        origin_set_level(level);
        EXPECT_EQ(origin_level(), level);
        if (level != ORIGIN_LOG_LEVEL_OFF) {
            EXPECT_TRUE(origin_should_log(level));
        } else {
            EXPECT_FALSE(origin_should_log(level));
        }
    }
}

TEST_F(TestRootLoggerC, set_level_failed_when_level_invalid)
{
    init_root_logger(test_info_);
    origin_set_level(INVALID_LEVEL_C);
    EXPECT_EQ(origin_level(), ORIGIN_LOG_LEVEL_TRACE);
}

TEST_F(TestRootLoggerC, set_level_failed_when_root_logger_nullptr)
{
    origin_set_level(ORIGIN_LOG_LEVEL_INFO);
}

TEST_F(TestRootLoggerC, get_level_failed_when_root_logger_nullptr)
{
    EXPECT_EQ(origin_level(), ORIGIN_LOG_LEVEL_OFF);
}

TEST_F(TestRootLoggerC, should_log_false_when_level_invalid)
{
    init_root_logger(test_info_);
    EXPECT_FALSE(origin_should_log(INVALID_LEVEL_C));
}

TEST_F(TestRootLoggerC, should_log_false_when_root_logger_nullptr)
{
    EXPECT_FALSE(origin_should_log(ORIGIN_LOG_LEVEL_OFF));
}

TEST_F(TestRootLoggerC, flush_on)
{
    init_root_logger(test_info_);
    for (const LogLevelC level : C_LOG_LEVELS) {
        origin_flush_on(level);
        EXPECT_EQ(origin_flush_level(), level);
        if (level != ORIGIN_LOG_LEVEL_OFF) {
            EXPECT_TRUE(origin_should_flush(level));
        } else {
            EXPECT_FALSE(origin_should_flush(level));
        }
    }
}

TEST_F(TestRootLoggerC, flush_on_failed_when_level_invalid)
{
    origin_flush_on(ORIGIN_LOG_LEVEL_TRACE);
}

TEST_F(TestRootLoggerC, flush_on_failed_when_root_logger_nullptr)
{
    init_root_logger(test_info_);
    origin_flush_on(ORIGIN_LOG_LEVEL_TRACE);
    EXPECT_EQ(origin_flush_level(), ORIGIN_LOG_LEVEL_TRACE);

    origin_flush_on(INVALID_LEVEL_C);
    EXPECT_EQ(origin_flush_level(), ORIGIN_LOG_LEVEL_TRACE);
}

TEST_F(TestRootLoggerC, should_flush_false_when_level_invalid)
{
    init_root_logger(test_info_);
    EXPECT_FALSE(origin_should_flush(INVALID_LEVEL_C));
}

TEST_F(TestRootLoggerC, should_flush_false_when_root_logger_nullptr)
{
    EXPECT_FALSE(origin_should_flush(INVALID_LEVEL_C));
}

TEST_F(TestRootLoggerC, fluash_level_failed_when_root_logger_nullptr)
{
    EXPECT_EQ(origin_flush_level(), ORIGIN_LOG_LEVEL_OFF);
}

TEST_F(TestRootLoggerC, set_pattern)
{
    init_root_logger(test_info_);

    origin_set_pattern("%v");
    for (uint32_t i = 0; i < 10; i++) {
        ORIGIN_LOGGING_ERROR("%u", i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestRootLoggerC, set_pattern_failed_when_pattern_nullptr)
{
    init_root_logger(test_info_);
    origin_set_pattern(nullptr);
    ORIGIN_LOGGING_ERROR("test");
    EXPECT_NE("test", _sink->buffer()[0]);
}

TEST_F(TestRootLoggerC, set_pattern_failed_when_pattern_empty)
{
    init_root_logger(test_info_);
    origin_set_pattern("");
    ORIGIN_LOGGING_ERROR("test");
    EXPECT_NE("test", _sink->buffer()[0]);
}

TEST_F(TestRootLoggerC, set_pattern_failed_when_root_logger_nullptr)
{
    origin_set_pattern("%v");
}

TEST_F(TestRootLoggerC, set_formatter)
{
    init_root_logger(test_info_);

    FormatterSt *formatter = origin_create_pattern_formatter("%v");
    origin_set_formatter(formatter);
    origin_destroy_formatter(formatter);

    for (uint32_t i = 0; i < 10; i++) {
        ORIGIN_LOGGING_ERROR("%u", i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestRootLoggerC, set_formatter_failed_when_formatter_nullptr)
{
    init_root_logger(test_info_);
    origin_set_formatter(nullptr);
    origin_set_formatter(_nullFormatter);
}

TEST_F(TestRootLoggerC, set_formatter_failed_when_root_logger_nullptr)
{
    FormatterSt *formatter = origin_create_pattern_formatter("%v");
    origin_set_formatter(formatter);
    origin_destroy_formatter(formatter);
}

TEST_F(TestRootLoggerC, log_failed_when_level_invalid)
{
    init_root_logger(test_info_);
    origin_log(__FILE__,
               __LINE__,
               __FUNCTION__,
               INVALID_LEVEL_C,
               "TestRootLoggerC.log_failed_when_level_invalid");
    EXPECT_EQ(_sink->buffer().size(), 0);
}

TEST_F(TestRootLoggerC, log_failed_when_formtat_nullptr)
{
    init_root_logger(test_info_);
    origin_log(__FILE__, __LINE__, __FUNCTION__, ORIGIN_LOG_LEVEL_ERROR, nullptr);
    EXPECT_EQ(_sink->buffer().size(), 0);
}

TEST_F(TestRootLoggerC, log_failed_when_root_logger_nullptr)
{
    origin_log(__FILE__,
               __LINE__,
               __FUNCTION__,
               ORIGIN_LOG_LEVEL_ERROR,
               "log_failed_when_root_logger_nullptr");
}

TEST_F(TestRootLoggerC, flush_failed_when_root_logger_nullptr)
{
    origin_flush();
}
}  // namespace logging_test
