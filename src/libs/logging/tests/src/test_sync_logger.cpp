#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "detail/common.hpp"
#include "detail/mock_sinks/log_content_buffer_sink.hpp"
#include "gtest/gtest.h"
#include "logging/formatters/formatter.hpp"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/log_source.hpp"
#include "logging/loggers/sync_logger.hpp"

using namespace logging_test;
using namespace origin::logging;

namespace logging_test {

class TestSyncLogger : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};

    void init_logger(const testing::TestInfo *test_info);
    std::shared_ptr<SyncLogger> _logger;
    std::shared_ptr<LogContentBufferSink> _sink = std::make_shared<LogContentBufferSink>();
};

void TestSyncLogger::init_logger(const testing::TestInfo *test_info)
{
    const std::string name = get_logger_name(test_info);
    _logger = std::make_shared<SyncLogger>(name, _sink);
}

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
    auto sinks = std::vector<std::shared_ptr<Sink>>{_sink, _sink, _sink};
    _logger = std::make_shared<SyncLogger>(name, sinks);
    EXPECT_EQ(_logger->name(), name);
    EXPECT_EQ(_logger->sinks().size(), sinks.size());
    EXPECT_EQ(_sink.use_count(), 2 * sinks.size() + 1);
}

TEST_F(TestSyncLogger, create_failed_when_name_empty)
{
    EXPECT_THROW(_logger = std::make_shared<SyncLogger>("", _sink), std::invalid_argument);
}

TEST_F(TestSyncLogger, create_failed_when_sink_nullptr)
{
    const std::string name = get_logger_name(test_info_);
    EXPECT_THROW(_logger = std::make_shared<SyncLogger>(name, nullptr), std::invalid_argument);
    auto sinkVector = std::vector<std::shared_ptr<Sink>>{_sink, nullptr};
    EXPECT_THROW(_logger = std::make_shared<SyncLogger>(name, sinkVector), std::invalid_argument);
    auto sinkList = std::initializer_list<std::shared_ptr<Sink>>{nullptr};
    EXPECT_THROW(_logger = std::make_shared<SyncLogger>(name, sinkList), std::invalid_argument);
}

TEST_F(TestSyncLogger, create_failed_when_sinks_empty)
{
    const std::string name = get_logger_name(test_info_);
    auto sinkVector = std::vector<std::shared_ptr<Sink>>();
    EXPECT_THROW(_logger = std::make_shared<SyncLogger>(name, sinkVector), std::invalid_argument);

    auto sinkList = std::vector<std::shared_ptr<Sink>>();
    EXPECT_THROW(_logger = std::make_shared<SyncLogger>(name, sinkList), std::invalid_argument);
}

TEST_F(TestSyncLogger, log_log)
{
    init_logger(test_info_);
    _sink->set_level(LogLevel::TRACE);
    for (const auto filterLevel : LOG_LEVELS) {
        _logger->set_level(filterLevel);
        for (const auto logLevel : LOG_LEVELS) {
            _logger->log(LOG_SRC_LOCAL, logLevel, "test");
        }
        if (filterLevel != LogLevel::OFF) {
            EXPECT_EQ(_sink->buffer().size(), diff_log_level(LogLevel::FATAL, _logger->level()) + 1)
                << log_level_string(filterLevel);
        } else {
            EXPECT_EQ(_sink->buffer().size(), 0);
        }
        _sink->clear();
    }
}

TEST_F(TestSyncLogger, log_flush)
{
    init_logger(test_info_);
    _sink->set_level(LogLevel::TRACE);
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
            _logger->log(LOG_SRC_LOCAL, level, i);
            if (!_logger->should_flush(level)) {
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
    init_logger(test_info_);
    _sink->set_level(LogLevel::TRACE);
    _logger->set_level(LogLevel::TRACE);
    constexpr uint32_t logCount = 100;
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

}  // namespace logging_test
