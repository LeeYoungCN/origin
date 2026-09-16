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
};

void TestRootLoggerC::SetUp()
{
    _sink->set_pattern("%v");
    _sink->set_level(LogLevel::TRACE);
    _sink->enable_debug_info(false);
}

void TestRootLoggerC::TearDown()
{
    destroy_mock_sink_st(_sinkSt);
    origin_destroy_logger(_loggerSt);
    origin_shutdown();
}

void TestRootLoggerC::init_root_logger(const testing::TestInfo *test_info)
{
    const std::string name = get_logger_name(test_info);
    _loggerSt = origin_create_sync_logger(name.data(), _sinks, 1);
    ASSERT_NE(_loggerSt, nullptr);
    origin_set_root_logger(_loggerSt);
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

TEST_F(TestRootLoggerC, set_root_logger_failed_when_nullptr)
{
    init_root_logger(test_info_);
    origin_set_root_logger(nullptr);
    const auto logger = create_mock_logger_st(nullptr);
    origin_set_root_logger(logger);
    destroy_mock_logger_st(logger);
}

TEST_F(TestRootLoggerC, log_filter)
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
            EXPECT_EQ(_sink->buffer().size(),
                      ORIGIN_LOG_LEVEL_FATAL - origin_logger_level(_loggerSt) + 1);
        } else {
            EXPECT_EQ(_sink->buffer().size(), 0);
        }
        _sink->clear();
    }
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
    origin_logger_flush(_loggerSt);
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), MAX_ITEM_CNT);
}

TEST_F(TestRootLoggerC, log_flush_on)
{
    init_root_logger(test_info_);

    origin_set_level(ORIGIN_LOG_LEVEL_TRACE);

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

TEST_F(TestRootLoggerC, log_macros)
{
    init_root_logger(test_info_);

    origin_set_level(ORIGIN_LOG_LEVEL_TRACE);

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

}  // namespace logging_test
