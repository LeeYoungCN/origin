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

protected:
    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>();
    LoggerSt *_loggerSt = nullptr;
    SinkSt *_sinkSt = create_mock_sink_st(_sink);
};

void TestSyncLoggerSt::SetUp()
{
    _sink->set_level(LogLevel::TRACE);
    _sink->enable_print_log(false);
}

void TestSyncLoggerSt::TearDown()
{
    destroy_mock_sink_st(_sinkSt);
    origin_destroy_logger(_loggerSt);
}

TEST_F(TestSyncLoggerSt, create_single_sink)
{
    const std::string name = get_logger_name(test_info_);
    SinkSt *sinks[] = {_sinkSt};
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, 1);
    EXPECT_EQ(_sink.use_count(), 3);
    EXPECT_EQ(origin_logger_name(_loggerSt), name);
    EXPECT_EQ(_sink->buffer().size(), 0);
}

TEST_F(TestSyncLoggerSt, create_multi_sinks)
{
    const std::string name = get_logger_name(test_info_);
    constexpr uint32_t sinkCnt = 3;
    SinkSt *sinks[sinkCnt] = {};
    for (SinkSt *&sink : sinks) {
        sink = _sinkSt;
    }
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, sinkCnt);
    EXPECT_EQ(origin_logger_name(_loggerSt), name);
    EXPECT_EQ(_sink.use_count(), sinkCnt + 2);
}

TEST_F(TestSyncLoggerSt, log_level)
{
    const std::string name = get_logger_name(test_info_);
    SinkSt const *sinks[] = {_sinkSt};
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, 1);

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

TEST_F(TestSyncLoggerSt, flush_level)
{
    const std::string name = get_logger_name(test_info_);
    SinkSt const *sinks[] = {_sinkSt};
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, 1);

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

TEST_F(TestSyncLoggerSt, log_log)
{
    const std::string name = get_logger_name(test_info_);
    SinkSt const *sinks[] = {_sinkSt};
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, 1);

    for (const LogLevelC filterLevel : C_LOG_LEVELS) {
        origin_logger_set_level(_loggerSt, filterLevel);
        for (const LogLevelC logLevel : C_LOG_LEVELS) {
            ORIGIN_LOGGER_LOG(
                _loggerSt, logLevel, "Level: [%s].", origin_log_level_full_string(logLevel));
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
    const std::string name = get_logger_name(test_info_);
    SinkSt const *sinks[] = {_sinkSt};
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, 1);

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
    const std::string name = get_logger_name(test_info_);
    SinkSt *sinks[] = {_sinkSt};
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, 1);

    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);

    for (LogLevelC const flushLevel : C_LOG_LEVELS) {
        // 设置刷新等级
        origin_logger_flush_on(_loggerSt, flushLevel);
        for (uint32_t i = 0; i < C_LOG_LEVELS.size(); ++i) {
            LogLevelC const level = C_LOG_LEVELS[i];
            if (level == ORIGIN_LOG_LEVEL_OFF) {
                break;
            }
            ORIGIN_LOGGER_LOG(
                _loggerSt, level, "Level: [%s].", origin_log_level_abbr_string(level));

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

TEST_F(TestSyncLoggerSt, log_function)
{
    const std::string name = get_logger_name(test_info_);
    SinkSt const *sinks[] = {_sinkSt};
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, 1);

    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);

    constexpr uint32_t logCount = 100;
    for (uint32_t i = 0; i < logCount; ++i) {
        ORIGIN_LOGGER_TRACE(_loggerSt,
                            "Level: [%s], idx: %u",
                            origin_log_level_full_string(ORIGIN_LOG_LEVEL_TRACE),
                            i);
        ORIGIN_LOGGER_DEBUG(_loggerSt,
                            "Level: [%s], idx: %u",
                            origin_log_level_full_string(ORIGIN_LOG_LEVEL_DEBUG),
                            i);
        ORIGIN_LOGGER_INFO(_loggerSt,
                           "Level: [%s], idx: %u",
                           origin_log_level_full_string(ORIGIN_LOG_LEVEL_INFO),
                           i);
        ORIGIN_LOGGER_WARN(_loggerSt,
                           "Level: [%s], idx: %u",
                           origin_log_level_full_string(ORIGIN_LOG_LEVEL_WARN),
                           i);
        ORIGIN_LOGGER_ERROR(_loggerSt,
                            "Level: [%s], idx: %u",
                            origin_log_level_full_string(ORIGIN_LOG_LEVEL_ERROR),
                            i);

        ORIGIN_LOGGER_FATAL(_loggerSt,
                            "Level: [%s], idx: %u",
                            origin_log_level_full_string(ORIGIN_LOG_LEVEL_FATAL),
                            i);
    }

    _sink->flush();
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), logCount * (LOG_LEVELS.size() - 1));
}

TEST_F(TestSyncLoggerSt, set_pattern)
{
    const std::string name = get_logger_name(test_info_);
    SinkSt const *sinks[] = {_sinkSt};
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, 1);
    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);

    origin_logger_set_pattern(_loggerSt, "%v");
    for (uint32_t i = 0; i < 100; i++) {
        ORIGIN_LOGGER_ERROR(_loggerSt, "%u", i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestSyncLoggerSt, set_formatter)
{
    const std::string name = get_logger_name(test_info_);
    SinkSt const *sinks[] = {_sinkSt};
    _loggerSt = origin_create_sync_logger(name.c_str(), sinks, 1);
    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);

    FormatterSt *formatter = origin_create_pattern_formatter("%v");
    origin_logger_set_formatter(_loggerSt, formatter);
    origin_destroy_formatter(formatter);

    for (uint32_t i = 0; i < 100; i++) {
        ORIGIN_LOGGER_ERROR(_loggerSt, "%u", i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

}  // namespace logging_test
