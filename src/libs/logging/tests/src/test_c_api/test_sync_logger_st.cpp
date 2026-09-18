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

class TestSyncLoggerSt : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;
    void init_logger(const testing::TestInfo *test_info);

    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>();
    LoggerSt *_loggerSt = nullptr;
    SinkSt *_sinkSt = create_mock_sink_st(_sink);
    SinkSt const *_sinks[1] = {_sinkSt};
};

void TestSyncLoggerSt::SetUp()
{
    _sink->set_pattern("%v");
    _sink->set_level(LogLevel::TRACE);
    _sink->enable_debug_info(false);
}

void TestSyncLoggerSt::TearDown()
{
    destroy_mock_sink_st(_sinkSt);
    origin_destroy_logger(_loggerSt);
}

void TestSyncLoggerSt::init_logger(const testing::TestInfo *test_info)
{
    const std::string name = get_logger_name(test_info);
    _loggerSt = origin_create_sync_logger(name.c_str(), _sinks, 1);
    ASSERT_NE(_loggerSt, nullptr);
}

TEST_F(TestSyncLoggerSt, create_failed_when_param_invalid)
{
    const std::string name = get_logger_name(test_info_);
    _loggerSt = origin_create_sync_logger(nullptr, _sinks, 1);
    EXPECT_EQ(_loggerSt, nullptr);
    _loggerSt = origin_create_sync_logger("", _sinks, 1);
    EXPECT_EQ(_loggerSt, nullptr);
    _loggerSt = origin_create_sync_logger(name.c_str(), nullptr, 1);
    EXPECT_EQ(_loggerSt, nullptr);
    _loggerSt = origin_create_sync_logger(name.c_str(), _sinks, 0);
    EXPECT_EQ(_loggerSt, nullptr);
    _sinks[0] = nullptr;
    _loggerSt = origin_create_sync_logger(name.c_str(), _sinks, 1);
    EXPECT_EQ(_loggerSt, nullptr);
}

TEST_F(TestSyncLoggerSt, create_single_sink)
{
    const std::string name = get_logger_name(test_info_);
    _loggerSt = origin_create_sync_logger(name.c_str(), _sinks, 1);
    ASSERT_NE(_loggerSt, nullptr);

    EXPECT_EQ(_sink.use_count(), 3);
    EXPECT_EQ(origin_logger_name(_loggerSt), name);
    EXPECT_EQ(_sink->buffer().size(), 0);
}

TEST_F(TestSyncLoggerSt, create_multi_sinks)
{
    const std::string name = get_logger_name(test_info_);
    constexpr uint32_t sinkCnt = 3;
    SinkSt *sinks[sinkCnt] = {};
    for (auto &sink : sinks) {
        sink = _sinkSt;
    }
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, sinkCnt);
    ASSERT_NE(_loggerSt, nullptr);
    EXPECT_EQ(origin_logger_name(_loggerSt), name);
    EXPECT_EQ(_sink.use_count(), sinkCnt + 2);
}

TEST_F(TestSyncLoggerSt, log_filter)
{
    init_logger(test_info_);
    for (const LogLevelC filterLevel : C_LOG_LEVELS) {
        origin_logger_set_level(_loggerSt, filterLevel);
        for (const LogLevelC logLevel : C_LOG_LEVELS) {
            ORIGIN_LOGGER_LOG(_loggerSt,
                              logLevel,
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

TEST_F(TestSyncLoggerSt, log_flush)
{
    init_logger(test_info_);

    constexpr uint32_t MAX_ITEM_CNT = 100;
    for (uint32_t i = 0; i < MAX_ITEM_CNT; ++i) {
        ORIGIN_LOGGER_ERROR(_loggerSt, "test log: %u", i);
        EXPECT_EQ(_sink->buffer().size(), i + 1);
        EXPECT_EQ(_sink->disk().size(), 0);
    }
    origin_logger_flush(_loggerSt);
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), MAX_ITEM_CNT);
}

TEST_F(TestSyncLoggerSt, log_flush_on)
{
    init_logger(test_info_);

    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);

    for (const LogLevelC  flushLevel : C_LOG_LEVELS) {
        // 设置刷新等级
        origin_logger_flush_on(_loggerSt, flushLevel);
        for (uint32_t i = 0; i < C_LOG_LEVELS.size(); ++i) {
            const LogLevelC  level = C_LOG_LEVELS[i];
            if (level == ORIGIN_LOG_LEVEL_OFF) {
                break;
            }
            ORIGIN_LOGGER_LOG(_loggerSt,
                              level,
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

TEST_F(TestSyncLoggerSt, log_macros)
{
    init_logger(test_info_);

    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);

    constexpr uint32_t logCount = 100;
    for (uint32_t i = 0; i < logCount; ++i) {
        ORIGIN_LOGGER_TRACE(_loggerSt,
                            "Level: [%s], idx: %u",
                            origin_log_level_full_name(ORIGIN_LOG_LEVEL_TRACE),
                            i);
        ORIGIN_LOGGER_DEBUG(_loggerSt,
                            "Level: [%s], idx: %u",
                            origin_log_level_full_name(ORIGIN_LOG_LEVEL_DEBUG),
                            i);
        ORIGIN_LOGGER_INFO(_loggerSt,
                           "Level: [%s], idx: %u",
                           origin_log_level_full_name(ORIGIN_LOG_LEVEL_INFO),
                           i);
        ORIGIN_LOGGER_WARN(_loggerSt,
                           "Level: [%s], idx: %u",
                           origin_log_level_full_name(ORIGIN_LOG_LEVEL_WARN),
                           i);
        ORIGIN_LOGGER_ERROR(_loggerSt,
                            "Level: [%s], idx: %u",
                            origin_log_level_full_name(ORIGIN_LOG_LEVEL_ERROR),
                            i);

        ORIGIN_LOGGER_FATAL(_loggerSt,
                            "Level: [%s], idx: %u",
                            origin_log_level_full_name(ORIGIN_LOG_LEVEL_FATAL),
                            i);
    }

    _sink->flush();
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), logCount * (LOG_LEVELS.size() - 1));
}

}  // namespace logging_test
