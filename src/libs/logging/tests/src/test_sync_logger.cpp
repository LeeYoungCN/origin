#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "detail/common.h"
#include "detail/log_content_buffer_sink.h"
#include "gtest/gtest.h"
#include "logging/formatters/formatter.h"
#include "logging/formatters/pattern_formatter.h"
#include "logging/log_level.h"
#include "logging/log_source.h"
#include "logging/loggers/sync_logger.h"

using namespace logging_test;
using namespace origin::logging;

namespace logging_test {

class TestSyncLogger : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};

protected:
    std::shared_ptr<SyncLogger> _logger;
    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>();
};

TEST_F(TestSyncLogger, create_single_sink)
{
    const std::string name = get_logger_name(test_info_);
    _logger = std::make_shared<SyncLogger>(name, _sink);
    EXPECT_EQ(_sink.use_count(), 2);
    EXPECT_EQ(_logger->name(), name);
    EXPECT_EQ(_logger->sinks().size(), 1);
    auto *sinkPtr = reinterpret_cast<LogContentBufferSink *>(_logger->sinks()[0].get());
    EXPECT_EQ(sinkPtr->buffer().size(), 0);
}

TEST_F(TestSyncLogger, create_initializer_list)
{
    const std::string name = get_logger_name(test_info_);
    auto sinks = std::initializer_list<std::shared_ptr<Sink>>{_sink, _sink, _sink};
    _logger = std::make_shared<SyncLogger>(name, sinks);
    EXPECT_EQ(_logger->name(), name);
    EXPECT_EQ(_logger->sinks().size(), sinks.size());
    EXPECT_EQ(_sink.use_count(), 2 * sinks.size() + 1);
}

TEST_F(TestSyncLogger, create_vector)
{
    const std::string name = get_logger_name(test_info_);
    auto sinks = std::vector<std::shared_ptr<Sink>>{_sink, _sink};
    sinks.push_back(_sink);
    _logger = std::make_shared<SyncLogger>(name, sinks);
    EXPECT_EQ(_logger->name(), name);
    EXPECT_EQ(_logger->sinks().size(), sinks.size());
    EXPECT_EQ(_sink.use_count(), 2 * sinks.size() + 1);
}

TEST_F(TestSyncLogger, log_level)
{
    const std::string name = get_logger_name(test_info_);
    _logger = std::make_shared<SyncLogger>(name, _sink);

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

TEST_F(TestSyncLogger, flush_level)
{
    const std::string name = get_logger_name(test_info_);
    _logger = std::make_shared<SyncLogger>(name, _sink);

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

TEST_F(TestSyncLogger, log_log)
{
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LOG_LEVELS.at(0));
    _logger = std::make_shared<SyncLogger>(name, _sink);
    for (auto filterLevel : LOG_LEVELS) {
        _logger->set_level(filterLevel);
        for (auto logLevel : LOG_LEVELS) {
            _logger->log(LOG_SRC_LOCAL, logLevel, "test");
        }
        if (filterLevel != LogLevel::OFF) {
            EXPECT_EQ(_sink->buffer().size(), LogLevel::FATAL - _logger->level() + 1)
                << log_level_to_string(filterLevel);
        } else {
            EXPECT_EQ(_sink->buffer().size(), 0);
        }
        _sink->clear();
    }
}

TEST_F(TestSyncLogger, log_flush)
{
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LOG_LEVELS.at(0));
    _logger = std::make_shared<SyncLogger>(name, _sink);
    constexpr uint32_t MAX_ITEM_CNT = 100;
    for (uint32_t i = 0; i < MAX_ITEM_CNT; ++i) {
        _logger->error(LOG_SRC_LOCAL, i);
        EXPECT_EQ(_sink->buffer().size(), i + 1);
        EXPECT_EQ(_sink->disk().size(), 0);
    }
    _logger->flush();
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), MAX_ITEM_CNT);
}

TEST_F(TestSyncLogger, log_flush_on)
{
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LOG_LEVELS.at(0));
    _logger = std::make_shared<SyncLogger>(name, _sink);
    _logger->set_level(LOG_LEVELS.at(0));

    for (auto flushLevel : LOG_LEVELS) {
        // 设置刷新等级
        _logger->flush_on(flushLevel);
        for (uint32_t i = 0; i < LOG_LEVELS.size(); ++i) {
            LogLevel level = LOG_LEVELS[i];
            if (level == LogLevel::OFF) {
                break;
            }
            _logger->log(LOG_SRC_LOCAL, level, i);
            if (_logger->flush_level() == LogLevel::OFF || level < _logger->flush_level()) {
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

TEST_F(TestSyncLogger, log_function)
{
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LOG_LEVELS.at(0));
    _logger = std::make_shared<SyncLogger>(name, _sink);
    _logger->set_level(LOG_LEVELS.at(0));
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
    }

    _sink->flush();
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), logCount * (LOG_LEVELS.size() - 1) * 4);
}

TEST_F(TestSyncLogger, set_pattern)
{
    const std::string name = get_logger_name(test_info_);
    _logger = std::make_shared<SyncLogger>(name, _sink);
    _logger->set_pattern("%v");
    for (uint32_t i = 0; i < 100; i++) {
        _logger->error(i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestSyncLogger, set_formatter)
{
    const std::string name = get_logger_name(test_info_);
    _sink->set_level(LOG_LEVELS.at(0));
    _logger = std::make_shared<SyncLogger>(name, _sink);
    _logger->set_level(LOG_LEVELS.at(0));
    std::unique_ptr<Formatter> formatter = std::make_unique<PatternFormatter>("%v");
    _logger->set_formatter(formatter);
    for (uint32_t i = 0; i < 100; i++) {
        _logger->error(i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

}  // namespace logging_test
