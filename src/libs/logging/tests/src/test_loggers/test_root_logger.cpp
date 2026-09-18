#include <memory>
#include <stdexcept>

#include "detail/common.hpp"
#include "detail/mock_sinks/log_content_buffer_sink.hpp"
#include "gtest/gtest.h"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/loggers/sync_logger.hpp"
#include "logging/logging.hpp"

using namespace logging_test;
using namespace origin::logging;

namespace logging_test {
class TestRootLogger : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;

    void init_logger(const testing::TestInfo* test_info_);
    std::string _name;
    std::shared_ptr<SyncLogger> _logger;
    std::shared_ptr<LogContentBufferSink> _sink = create_sink<LogContentBufferSink>();
};

void TestRootLogger::SetUp()
{
    remove_all();
    _sink->enable_debug_info(false);
}

void TestRootLogger::TearDown()
{
    _sink->clear();
    remove_all();
}

void TestRootLogger::init_logger(const testing::TestInfo* test_info_)
{
    _name = get_logger_name(test_info_);
    _logger = create_logger<SyncLogger>(_name, _sink);
    set_root_logger(_logger);
    set_level(LogLevel::TRACE);
    _sink->set_level(LogLevel::TRACE);
}

TEST_F(TestRootLogger, set_and_get_root_logger)
{
    init_logger(test_info_);
    EXPECT_EQ(root_logger()->name(), _logger->name());
    EXPECT_EQ(level(), _logger->level());
    EXPECT_EQ(root_logger()->level(), _logger->level());
}

TEST_F(TestRootLogger, get_root_logger_failed_when_nullptr)
{
    EXPECT_EQ(root_logger(), nullptr);
    EXPECT_EQ(root_logger_raw(), nullptr);
}

TEST_F(TestRootLogger, log_level_filter)
{
    init_logger(test_info_);
    for (const LogLevel filterLevel : LOG_LEVELS) {
        set_level(filterLevel);
        for (const LogLevel logLevel : LOG_LEVELS) {
            log(LOG_SRC_LOCAL,
                logLevel,
                "FileterLevel: {}, Level: {}.",
                log_level_full_name(filterLevel),
                log_level_full_name(logLevel));
        }
        if (filterLevel != LogLevel::OFF) {
            EXPECT_EQ(_sink->buffer().size(), diff_log_level(LogLevel::FATAL, level()) + 1)
                << log_level_full_name(filterLevel);
        } else {
            EXPECT_EQ(_sink->buffer().size(), 0);
        }
        _sink->clear();
    }
}

TEST_F(TestRootLogger, flush_level_filter)
{
    init_logger(test_info_);
    for (const LogLevel flushLevel : LOG_LEVELS) {
        // 设置刷新等级
        flush_on(flushLevel);
        for (uint32_t i = 0; i < LOG_LEVELS.size(); ++i) {
            const LogLevel level = LOG_LEVELS[i];
            if (level == LogLevel::OFF) {
                break;
            }
            log(LOG_SRC_LOCAL, level, i);
            if (!should_flush(level)) {
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

TEST_F(TestRootLogger, flush)
{
    init_logger(test_info_);
    constexpr uint32_t MAX_ITEM_CNT = 100;
    for (uint32_t i = 0; i < MAX_ITEM_CNT; ++i) {
        error(LOG_SRC_LOCAL, i);
        EXPECT_EQ(_sink->buffer().size(), i + 1);
        EXPECT_EQ(_sink->disk().size(), 0);
    }
    flush();
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), MAX_ITEM_CNT);
}

TEST_F(TestRootLogger, log_function)
{
    init_logger(test_info_);
    constexpr uint32_t logCount = 100;
    for (uint32_t i = 0; i < logCount; ++i) {
        // trace
        trace(LOG_SRC_LOCAL, "{}", i);
        trace("{}", i);
        trace(LOG_SRC_LOCAL, i);
        trace(i);
        // debug
        debug(LOG_SRC_LOCAL, "{}", i);
        debug("{}", i);
        debug(LOG_SRC_LOCAL, i);
        debug(i);
        // info
        info(LOG_SRC_LOCAL, "{}", i);
        info("{}", i);
        info(LOG_SRC_LOCAL, i);
        info(i);
        // warn
        warn(LOG_SRC_LOCAL, "{}", i);
        warn("{}", i);
        warn(LOG_SRC_LOCAL, i);
        warn(i);
        // error
        error(LOG_SRC_LOCAL, "{}", i);
        error("{}", i);
        error(LOG_SRC_LOCAL, i);
        error(i);
        // fatal
        fatal(LOG_SRC_LOCAL, "{}", i);
        fatal("{}", i);
        fatal(LOG_SRC_LOCAL, i);
        fatal(i);
    }

    flush();
    EXPECT_EQ(_sink->buffer().size(), 0);
    EXPECT_EQ(_sink->disk().size(), logCount * (LOG_LEVELS.size() - 1) * 4);
}

TEST_F(TestRootLogger, log_level)
{
    init_logger(test_info_);
    for (const LogLevel logLevel : LOG_LEVELS) {
        set_level(logLevel);
        EXPECT_EQ(level(), logLevel);
        if (logLevel != LogLevel::OFF) {
            EXPECT_TRUE(should_log(logLevel));
        } else {
            EXPECT_FALSE(should_log(logLevel));
        }
    }
}

TEST_F(TestRootLogger, set_level_failed_when_root_logger_nullptr)
{
    EXPECT_THROW(set_level(LogLevel::INFO), std::runtime_error);
}

TEST_F(TestRootLogger, level_failed_when_root_logger_nullptr)
{
    EXPECT_THROW(level(), std::runtime_error);
}

TEST_F(TestRootLogger, should_log_failed_when_root_logger_nullptr)
{
    EXPECT_THROW(should_log(LogLevel::INFO), std::runtime_error);
}

TEST_F(TestRootLogger, flush_on)
{
    init_logger(test_info_);
    for (LogLevel const level : LOG_LEVELS) {
        flush_on(level);
        EXPECT_EQ(flush_level(), level);
        if (level != LogLevel::OFF) {
            EXPECT_TRUE(should_flush(level));
        } else {
            EXPECT_FALSE(should_flush(level));
        }
    }
}

TEST_F(TestRootLogger, flush_on_failed_when_root_logger_nullptr)
{
    EXPECT_THROW(flush_on(LogLevel::INFO), std::runtime_error);
}

TEST_F(TestRootLogger, flush_level_failed_when_root_logger_nullptr)
{
    EXPECT_THROW(flush_level(), std::runtime_error);
}

TEST_F(TestRootLogger, should_flush_failed_when_root_logger_nullptr)
{
    EXPECT_THROW(should_flush(LogLevel::INFO), std::runtime_error);
}

TEST_F(TestRootLogger, set_pattern)
{
    init_logger(test_info_);
    set_pattern("%v");
    for (uint32_t i = 0; i < 100; i++) {
        error(i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestRootLogger, set_pattern_failed_when_empty)
{
    init_logger(test_info_);
    set_pattern("%v");
    error("test");
    EXPECT_EQ("test", _sink->buffer()[0]);
    set_pattern("");
    error("test");
    EXPECT_EQ("test", _sink->buffer()[1]);
}

TEST_F(TestRootLogger, set_pattern_failed_when_root_logger_nullptr)
{
    EXPECT_THROW(set_pattern("%v"), std::runtime_error);
}

TEST_F(TestRootLogger, set_formatter)
{
    init_logger(test_info_);
    const std::unique_ptr<Formatter> formatter = std::make_unique<PatternFormatter>("%v");
    set_formatter(formatter);
    for (uint32_t i = 0; i < 100; i++) {
        error(i);
        EXPECT_EQ(std::to_string(i), _sink->buffer()[i]);
    }
}

TEST_F(TestRootLogger, set_formatter_failed_when_formatter_nullptr)
{
    init_logger(test_info_);
    set_pattern("%v");
    error("test");
    EXPECT_EQ("test", _sink->buffer()[0]);
    set_formatter(nullptr);
    error("test");
    EXPECT_EQ("test", _sink->buffer()[1]);
}

TEST_F(TestRootLogger, set_formatter_failed_when_root_logger_nullptr)
{
    const std::unique_ptr<Formatter> formatter = std::make_unique<PatternFormatter>("%v");
    EXPECT_THROW(set_formatter(formatter), std::runtime_error);
}
}  // namespace logging_test
