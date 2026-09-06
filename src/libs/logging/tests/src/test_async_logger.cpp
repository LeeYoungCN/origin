#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "detail/common.h"
#include "detail/log_content_buffer_sink.h"
#include "gtest/gtest.h"
#include "internal/common.h"
#include "logging/formatters/formatter.h"
#include "logging/formatters/pattern_formatter.h"
#include "logging/log_level.h"
#include "logging/log_source.h"
#include "logging/loggers/async_logger.h"
#include "logging/logging.h"
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
    void wait_flush_complete(uint32_t expectedCount);

    std::shared_ptr<Logger> _logger;
    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>();
    std::shared_ptr<TaskPool> _taskPool = create_task_pool(1024, 4);
};

void TestAsyncLogger::wait_flush_complete(uint32_t expectedCount)
{
    constexpr uint32_t maxWaitTimeMs = 5000;
    uint32_t waitedTimeMs = 0;
    while (_sink->disk().size() < expectedCount) {
        sleep_ms(1);
        waitedTimeMs += 1;
        if (waitedTimeMs >= maxWaitTimeMs) {
            FAIL() << "Timeout waiting for log entries. Expected: " << expectedCount
                   << ", Actual: " << _sink->disk().size();
        }
    }
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), expectedCount);
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

TEST_F(TestAsyncLogger, log_level)
{
    const std::string name = get_logger_name(test_info_);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);

    for (LogLevel level : LOG_LEVELS) {
        _logger->set_level(level);
        EXPECT_EQ(_logger->level(), level);
        if (level != LogLevel::OFF) {
            EXPECT_TRUE(_logger->should_log(level));
        } else {
            EXPECT_FALSE(_logger->should_log(level));
        }
    }
}

TEST_F(TestAsyncLogger, flush_level)
{
    const std::string name = get_logger_name(test_info_);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);

    for (LogLevel level : LOG_LEVELS) {
        _logger->flush_on(level);
        EXPECT_EQ(_logger->flush_level(), level);
        if (level != LogLevel::OFF) {
            EXPECT_TRUE(_logger->should_flush(level));
        } else {
            EXPECT_FALSE(_logger->should_flush(level));
        }
    }
}

TEST_F(TestAsyncLogger, log_log)
{
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LogLevel::TRACE);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);

    for (auto filterLevel : LOG_LEVELS) {
        _sink->clear();
        _logger->set_level(filterLevel);

        for (auto logLevel : LOG_LEVELS) {
            _logger->log(LOG_SRC_LOCAL,
                         logLevel,
                         "fileLevel: {}, logLevel: {}.",
                         log_level_to_string(filterLevel),
                         log_level_to_string(logLevel));
            sleep_ms(1);
        }

        _logger->flush();

        if (filterLevel != LogLevel::OFF) {
            wait_flush_complete(LogLevel::FATAL - filterLevel + 1);
            EXPECT_EQ(_sink->disk().size(), LogLevel::FATAL - filterLevel + 1)
                << log_level_to_string(filterLevel);
        } else {
            EXPECT_EQ(_sink->disk().size(), 0);
        }
    }
}

TEST_F(TestAsyncLogger, log_flush)
{
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LogLevel::TRACE);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);
    constexpr uint32_t logCount = 100;
    for (uint32_t i = 0; i < logCount; ++i) {
        _logger->error(LOG_SRC_LOCAL, i);
        sleep_ms(1);
    }

    _logger->flush();

    wait_flush_complete(logCount);
}

TEST_F(TestAsyncLogger, log_function)
{
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LogLevel::TRACE);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);
    _logger->set_level(LogLevel::TRACE);
    uint32_t logCount = 100;
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

TEST_F(TestAsyncLogger, set_pattern)
{
    const std::string name = get_logger_name(test_info_);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);
    _logger->set_pattern("%v");
    constexpr uint32_t logCount = 100;
    for (uint32_t i = 0; i < logCount; i++) {
        _logger->error(i);
        sleep_ms(1);
    }
    _logger->flush();
    wait_flush_complete(logCount);
    for (uint32_t i = 0; i < logCount; i++) {
        EXPECT_EQ(std::to_string(i), _sink->disk()[i]);
    }
}

TEST_F(TestAsyncLogger, set_formatter)
{
    constexpr uint32_t logCount = 50;
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LogLevel::TRACE);
    _logger = std::make_shared<AsyncLogger>(name, _sink, _taskPool);
    _logger->set_level(LogLevel::TRACE);
    std::unique_ptr<Formatter> formatter = std::make_unique<PatternFormatter>("%v");
    _logger->set_formatter(formatter);
    for (uint32_t i = 0; i < logCount; i++) {
        _logger->error(i);
        sleep_ms(1);
    }
    _logger->flush();
    wait_flush_complete(logCount);
    for (uint32_t i = 0; i < logCount; i++) {
        EXPECT_EQ(std::to_string(i), _sink->disk()[i]);
    }
}

}  // namespace logging_test
