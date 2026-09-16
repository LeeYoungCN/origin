#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <vector>

#include "common/debug/debug_logger.h"
#include "detail/common.hpp"
#include "detail/mock_sinks/log_content_buffer_sink.hpp"
#include "gtest/gtest.h"
#include "logging/c/logging_c.h"
#include "logging/log_level.hpp"
#include "utils/date_time_utils.h"

using namespace origin::logging;
using namespace origin::date_time;

namespace logging_test {

class TestAsyncLoggerSt : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;
    void wait_flush_complete(uint32_t expectedCount) const;
    void wait_log_complete(uint32_t expectedCount) const;
    void init_logger(const testing::TestInfo *test_info);

protected:
    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>();
    LoggerSt *_loggerSt = nullptr;
    SinkSt *_sinkSt = create_mock_sink_st(_sink);
    TaskPoolSt *_taskPoolSt = origin_create_task_pool(1024, 3);
    SinkSt *_sinks[1] = {_sinkSt};
};

void TestAsyncLoggerSt::SetUp()
{
    _sink->set_pattern("%v");
    _sink->set_level(LogLevel::TRACE);
    _sink->enable_debug_info(false);
}

void TestAsyncLoggerSt::TearDown()
{
    destroy_mock_sink_st(_sinkSt);
    origin_destroy_logger(_loggerSt);
    origin_destroy_task_pool(_taskPoolSt);
}

void TestAsyncLoggerSt::init_logger(const testing::TestInfo *test_info)
{
    const std::string name = get_logger_name(test_info);
    _loggerSt = origin_create_async_logger(name.c_str(), _sinks, 1, _taskPoolSt);
    ASSERT_NE(_loggerSt, nullptr);
}

void TestAsyncLoggerSt::wait_log_complete(uint32_t expectedCount) const
{
    uint32_t waitedTimeMs = 0;
    while (_sink->buffer().size() < expectedCount) {
        constexpr uint32_t interval = 10;
        sleep_ms(1);
        waitedTimeMs += interval;
        if (constexpr uint32_t maxWaitTimeMs = 5000; waitedTimeMs >= maxWaitTimeMs) {
            ORIGIN_DEBUG_ERR("Wait log finish. Timeout: {}, expected: {}, disk: {}, buffer: {}.",
                             waitedTimeMs,
                             expectedCount,
                             _sink->disk().size(),
                             _sink->buffer().size());
            FAIL();
        }
    }
}

void TestAsyncLoggerSt::wait_flush_complete(uint32_t expectedCount) const
{
    uint32_t waitedTimeMs = 0;
    while (_sink->disk().size() < expectedCount) {
        constexpr uint32_t interval = 10;
        sleep_ms(interval);
        waitedTimeMs += interval;
        if (constexpr uint32_t maxWaitTimeMs = 5000; waitedTimeMs >= maxWaitTimeMs) {
            ORIGIN_DEBUG_ERR("Wait flush finish. Timeout: {}, expected: {}, disk: {}, buffer: {}.",
                             waitedTimeMs,
                             expectedCount,
                             _sink->disk().size(),
                             _sink->buffer().size());
            FAIL();
        }
    }
}

TEST_F(TestAsyncLoggerSt, create_failed_when_param_invalid)
{
    const std::string name = get_logger_name(test_info_);
    _loggerSt = origin_create_async_logger(nullptr, _sinks, 1, _taskPoolSt);
    EXPECT_EQ(_loggerSt, nullptr);
    _loggerSt = origin_create_async_logger("", _sinks, 1, _taskPoolSt);
    EXPECT_EQ(_loggerSt, nullptr);
    _loggerSt = origin_create_async_logger(name.c_str(), nullptr, 1, _taskPoolSt);
    EXPECT_EQ(_loggerSt, nullptr);
    _loggerSt = origin_create_async_logger(name.c_str(), _sinks, 0, _taskPoolSt);
    EXPECT_EQ(_loggerSt, nullptr);
    _loggerSt = origin_create_async_logger(name.c_str(), _sinks, 1, nullptr);
    EXPECT_EQ(_loggerSt, nullptr);
    _sinks[0] = nullptr;
    _loggerSt = origin_create_async_logger(name.c_str(), _sinks, 1, _taskPoolSt);
    EXPECT_EQ(_loggerSt, nullptr);
}

TEST_F(TestAsyncLoggerSt, create_single_sink)
{
    const std::string name = get_logger_name(test_info_);

    _loggerSt = origin_create_async_logger(name.c_str(), _sinks, 1, _taskPoolSt);
    ASSERT_NE(_loggerSt, nullptr);
    EXPECT_EQ(_sink.use_count(), 3);
    EXPECT_EQ(origin_logger_name(_loggerSt), name);
    EXPECT_EQ(_sink->buffer().size(), 0);
}

TEST_F(TestAsyncLoggerSt, create_multi_sinks)
{
    const std::string name = get_logger_name(test_info_);
    constexpr uint32_t sinkCnt = 3;
    SinkSt *sinks[sinkCnt] = {};
    for (SinkSt *&sink : sinks) {
        sink = _sinkSt;
    }
    _loggerSt = origin_create_async_logger(name.c_str(), sinks, sinkCnt, _taskPoolSt);
    EXPECT_EQ(origin_logger_name(_loggerSt), name);
    EXPECT_EQ(_sink.use_count(), sinkCnt + 2);
}

TEST_F(TestAsyncLoggerSt, create_by_root_task_pool)
{
    origin_set_root_task_pool(_taskPoolSt);
    const std::string name = get_logger_name(test_info_);
    _loggerSt = origin_create_async_logger(name.c_str(), _sinks, 1, nullptr);
    ASSERT_NE(_loggerSt, nullptr);
    EXPECT_EQ(_sink.use_count(), 3);
    EXPECT_EQ(origin_logger_name(_loggerSt), name);
    EXPECT_EQ(_sink->buffer().size(), 0);
    origin_shutdown();
}

TEST_F(TestAsyncLoggerSt, log_filter)
{
    init_logger(test_info_);

    for (const LogLevelC filterLevel : C_LOG_LEVELS) {
        _sink->clear();
        origin_logger_set_level(_loggerSt, filterLevel);

        for (const LogLevelC logLevel : C_LOG_LEVELS) {
            ORIGIN_LOGGER_LOG(_loggerSt,
                              logLevel,
                              "FilterLevel: [%s], Level: [%s].",
                              origin_log_level_full_name(filterLevel),
                              origin_log_level_full_name(logLevel));
            sleep_ms(1);
        }

        if (filterLevel != ORIGIN_LOG_LEVEL_OFF) {
            uint32_t const expectSize = ORIGIN_LOG_LEVEL_FATAL - filterLevel + 1;
            wait_log_complete(expectSize);
            EXPECT_EQ(_sink->buffer().size(), expectSize)
                << origin_log_level_full_name(filterLevel);
        } else {
            EXPECT_EQ(_sink->buffer().size(), 0);
        }
    }
}

TEST_F(TestAsyncLoggerSt, log_flush)
{
    init_logger(test_info_);

    constexpr uint32_t logCount = 100;
    for (uint32_t i = 0; i < logCount; ++i) {
        ORIGIN_LOGGER_ERROR(_loggerSt, "%u", i);
        sleep_ms(1);
    }

    origin_logger_flush(_loggerSt);
    wait_flush_complete(logCount);
}

TEST_F(TestAsyncLoggerSt, log_flush_on)
{
    init_logger(test_info_);
    origin_logger_set_level(_loggerSt, ORIGIN_LOG_LEVEL_TRACE);

    for (LogLevelC const flushLevel : C_LOG_LEVELS) {
        // 设置刷新等级
        origin_logger_flush_on(_loggerSt, flushLevel);
        for (uint32_t i = 0; i < C_LOG_LEVELS.size(); ++i) {
            LogLevelC const level = C_LOG_LEVELS[i];
            if (level == ORIGIN_LOG_LEVEL_OFF) {
                break;
            }
            std::string const logContent = std::format("FlushLevel: [{}], Level: [{}].",
                                                       origin_log_level_full_name(flushLevel),
                                                       origin_log_level_full_name(level));
            ORIGIN_LOGGER_LOG(_loggerSt, level, "%s", logContent.c_str());

            if (!origin_logger_should_flush(_loggerSt, level)) {
                EXPECT_LE(_sink->buffer().size(), i + 1);
                EXPECT_EQ(_sink->disk().size(), 0);
            } else {
                wait_flush_complete(i + 1);
                EXPECT_EQ(_sink->buffer().size(), 0);
                EXPECT_EQ(_sink->disk().size(), i + 1) << logContent;
            }
            sleep_ms(1);
        }
        _sink->clear();
    }
}

TEST_F(TestAsyncLoggerSt, log_macros)
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

    constexpr auto expectCnt = logCount * (ORIGIN_LOG_LEVEL_FATAL - ORIGIN_LOG_LEVEL_TRACE + 1);
    wait_log_complete(expectCnt);
    EXPECT_EQ(_sink->buffer().size(), expectCnt);
    EXPECT_EQ(_sink->disk().size(), 0);

    origin_logger_flush(_loggerSt);
    wait_flush_complete(expectCnt);
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), expectCnt);
}

}  // namespace logging_test
