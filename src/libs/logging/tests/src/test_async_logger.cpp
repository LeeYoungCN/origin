#include <cstdint>
#include <format>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "common/debug/debug_logger.h"
#include "detail/common.hpp"
#include "detail/mock_sinks/log_content_buffer_sink.hpp"
#include "gtest/gtest.h"
#include "logging/log_level.hpp"
#include "logging/log_source.hpp"
#include "logging/loggers/async_logger.hpp"
#include "logging/logging.hpp"
#include "utils/date_time_utils.h"

using namespace logging_test;
using namespace origin::date_time;
using namespace origin::logging;

namespace logging_test {

class TestAsyncLogger : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};

protected:
    void wait_flush_complete(uint32_t expectedCount) const;
    void wait_log_complete(uint32_t expectedCount) const;
    void init_logger(const testing::TestInfo *test_info);

    std::shared_ptr<Logger> _logger;
    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>(2048);
    std::shared_ptr<TaskPool> _taskPool = create_task_pool(1024, 3);
    const uint32_t _interval = 1;
    const uint32_t _maxWaitTimeMs = 5000;
};

void TestAsyncLogger::init_logger(const testing::TestInfo *test_info)
{
    const std::string name = get_logger_name(test_info);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);
}

void TestAsyncLogger::wait_log_complete(uint32_t expectedCount) const
{
    uint32_t waitedTimeMs = 0;
    while (_sink->buffer().size() < expectedCount) {
        sleep_ms(_interval);
        waitedTimeMs += _interval;
        if (waitedTimeMs >= _maxWaitTimeMs) {
            ORIGIN_DEBUG_ERR("Wait log finish. Timeout: {}, expected: {}, disk: {}, buffer: {}.",
                             waitedTimeMs,
                             expectedCount,
                             _sink->disk().size(),
                             _sink->buffer().size());
            FAIL();
        }
    }
}

void TestAsyncLogger::wait_flush_complete(uint32_t expectedCount) const
{
    uint32_t waitedTimeMs = 0;
    while (_sink->disk().size() < expectedCount) {
        sleep_ms(_interval);
        waitedTimeMs += _interval;
        if (waitedTimeMs >= _maxWaitTimeMs) {
            ORIGIN_DEBUG_ERR("Wait flush finish. Timeout: {}, expected: {}, disk: {}, buffer: {}.",
                             waitedTimeMs,
                             expectedCount,
                             _sink->disk().size(),
                             _sink->buffer().size());
            FAIL();
        }
    }
}

TEST_F(TestAsyncLogger, create_single_sink)
{
    const std::string name = get_logger_name(test_info_);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);
    EXPECT_EQ(_sink.use_count(), 2);
    EXPECT_EQ(_logger->name(), name);
    EXPECT_EQ(_logger->sinks().size(), 1);
    auto *sinkPtr = reinterpret_cast<LogContentBufferSink *>(_logger->sinks()[0].get());
    EXPECT_EQ(sinkPtr->buffer().size(), 0);
}

TEST_F(TestAsyncLogger, create_initializer_list)
{
    const std::string name = get_logger_name(test_info_);
    auto sinks = std::initializer_list<std::shared_ptr<Sink>>{_sink, _sink, _sink};
    _logger = std::make_shared<AsyncLogger>(name, sinks, _taskPool);
    EXPECT_EQ(_logger->name(), name);
    EXPECT_EQ(_logger->sinks().size(), sinks.size());
    EXPECT_EQ(_sink.use_count(), 2 * sinks.size() + 1);
}

TEST_F(TestAsyncLogger, create_vector)
{
    const std::string name = get_logger_name(test_info_);
    auto sinks = std::vector<std::shared_ptr<Sink>>{_sink, _sink};
    sinks.push_back(_sink);
    _logger = std::make_shared<AsyncLogger>(name, sinks, _taskPool);
    EXPECT_EQ(_logger->name(), name);
    EXPECT_EQ(_logger->sinks().size(), sinks.size());
    EXPECT_EQ(_sink.use_count(), 2 * sinks.size() + 1);
}

TEST_F(TestAsyncLogger, create_failed_when_name_empty)
{
    EXPECT_THROW(_logger = std::make_shared<AsyncLogger>("", _sink, _taskPool),
                 std::invalid_argument);
}

TEST_F(TestAsyncLogger, create_failed_when_sink_nullptr)
{
    const std::string name = get_logger_name(test_info_);
    EXPECT_THROW(_logger = std::make_shared<AsyncLogger>(name, nullptr, _taskPool),
                 std::invalid_argument);
    auto sinkVector = std::vector<std::shared_ptr<Sink>>{_sink, nullptr};
    EXPECT_THROW(_logger = std::make_shared<AsyncLogger>(name, sinkVector, _taskPool),
                 std::invalid_argument);
    auto sinkList = std::initializer_list<std::shared_ptr<Sink>>{nullptr};
    EXPECT_THROW(_logger = std::make_shared<AsyncLogger>(name, sinkList, _taskPool),
                 std::invalid_argument);
}

TEST_F(TestAsyncLogger, create_failed_when_sinks_empty)
{
    const std::string name = get_logger_name(test_info_);
    auto sinkVector = std::vector<std::shared_ptr<Sink>>();
    EXPECT_THROW(_logger = std::make_shared<AsyncLogger>(name, sinkVector, _taskPool),
                 std::invalid_argument);

    auto sinkList = std::vector<std::shared_ptr<Sink>>();
    EXPECT_THROW(_logger = std::make_shared<AsyncLogger>(name, sinkList, _taskPool),
                 std::invalid_argument);
}

TEST_F(TestAsyncLogger, create_failed_when_task_pool_not_registered)
{
    const std::string name = get_logger_name(test_info_);
    EXPECT_THROW(_logger = std::make_shared<AsyncLogger>(name, _sink), std::runtime_error);
}

TEST_F(TestAsyncLogger, create_failed_when_task_pool_nullptr)
{
    const std::string name = get_logger_name(test_info_);
    _taskPool.reset();
    EXPECT_THROW(_logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool),
                 std::invalid_argument);
}

TEST_F(TestAsyncLogger, log_filter)
{
    init_logger(test_info_);
    _sink->set_level(LogLevel::TRACE);

    for (const auto filterLevel : LOG_LEVELS) {
        _sink->clear();
        _logger->set_level(filterLevel);

        for (const auto logLevel : LOG_LEVELS) {
            _logger->log(LOG_SRC_LOCAL,
                         logLevel,
                         "fileLevel: {}, logLevel: {}.",
                         log_level_full_name(filterLevel),
                         log_level_full_name(logLevel));
            sleep_ms(1);
        }

        if (filterLevel != LogLevel::OFF) {
            wait_log_complete(static_cast<uint32_t>(diff_log_level(LogLevel::FATAL, filterLevel)) +
                              1);
            EXPECT_EQ(_sink->buffer().size(), diff_log_level(LogLevel::FATAL, filterLevel) + 1)
                << log_level_full_name(filterLevel);
        } else {
            EXPECT_EQ(_sink->buffer().size(), 0);
        }
    }
}

TEST_F(TestAsyncLogger, log_flush)
{
    init_logger(test_info_);
    _sink->set_level(LogLevel::TRACE);
    constexpr uint32_t logCount = 100;
    for (uint32_t i = 0; i < logCount; ++i) {
        _logger->error(LOG_SRC_LOCAL, i);
        sleep_ms(1);
    }

    _logger->flush();

    wait_flush_complete(logCount);
}

TEST_F(TestAsyncLogger, log_flush_on)
{
    init_logger(test_info_);
    _sink->set_level(LogLevel::TRACE);
    _logger->set_level(LogLevel::TRACE);

    for (const auto flushLevel : LOG_LEVELS) {
        // 设置刷新等级
        _logger->flush_on(flushLevel);
        for (uint32_t i = 0; i < LOG_LEVELS.size(); ++i) {
            const LogLevel level = LOG_LEVELS[i];
            if (level == LogLevel::OFF) {
                break;
            }
            std::string const logMsg = std::format("FlushLevel: {}, Level: {}.",
                                                   log_level_full_name(flushLevel),
                                                   log_level_full_name(level));
            _logger->log(LOG_SRC_LOCAL, level, logMsg);
            if (!_logger->should_flush(level)) {
                EXPECT_LE(_sink->buffer().size(), i + 1);
                EXPECT_EQ(_sink->disk().size(), 0);
            } else {
                wait_flush_complete(i + 1);
                EXPECT_EQ(_sink->buffer().size(), 0);
                EXPECT_EQ(_sink->disk().size(), i + 1) << logMsg;
            }
            sleep_ms(1);
        }
        _sink->clear();
    }
}

TEST_F(TestAsyncLogger, log_function)
{
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LogLevel::TRACE);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);
    _logger->set_level(LogLevel::TRACE);
    constexpr uint32_t logCount = 50;
    for (uint32_t i = 0; i < logCount; ++i) {
        // trace
        _logger->trace(LOG_SRC_LOCAL, "{}", i);
        _logger->trace("{}", i);
        _logger->trace(LOG_SRC_LOCAL, i);
        _logger->trace(i);
        // debug
        _logger->debug(LOG_SRC_LOCAL, "{}", i);
        _logger->debug("{}", i);
        _logger->debug(LOG_SRC_LOCAL, i);
        _logger->debug(i);
        // info
        _logger->info(LOG_SRC_LOCAL, "{}", i);
        _logger->info("{}", i);
        _logger->info(LOG_SRC_LOCAL, i);
        _logger->info(i);
        // warn
        _logger->warn(LOG_SRC_LOCAL, "{}", i);
        _logger->warn("{}", i);
        _logger->warn(LOG_SRC_LOCAL, i);
        _logger->warn(i);
        // error
        _logger->warn(LOG_SRC_LOCAL, "{}", i);
        _logger->warn("{}", i);
        _logger->warn(LOG_SRC_LOCAL, i);
        _logger->warn(i);
        // fatal
        _logger->fatal(LOG_SRC_LOCAL, "{}", i);
        _logger->fatal("{}", i);
        _logger->fatal(LOG_SRC_LOCAL, i);
        _logger->fatal(i);
        sleep_ms(1);
    }
    _logger->flush();
    wait_flush_complete(logCount * static_cast<uint32_t>(LOG_LEVELS.size() - 1) * 4);
}

}  // namespace logging_test
