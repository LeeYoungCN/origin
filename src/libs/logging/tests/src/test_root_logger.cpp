#include <memory>

#include "detail/common.h"
#include "detail/log_content_buffer_sink.h"
#include "gtest/gtest.h"
#include "logging/formatters/pattern_formatter.h"
#include "logging/loggers/sync_logger.h"
#include "logging/logging.h"

using namespace logging_test;
using namespace origin::logging;

namespace logging_test {
class TestRootLogger : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override;
};

void TestRootLogger::TearDown()
{
    shutdown();
}

TEST_F(TestRootLogger, should_log_and_set_level)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, LogContentBufferSink>(name);
    set_root_logger(logger);
    EXPECT_EQ(root_logger()->name(), name);
    EXPECT_EQ(root_logger_raw()->name(), name);

    for (LogLevel level : LOG_LEVELS) {
        set_level(level);
        EXPECT_EQ(root_logger()->level(), level);
        if (level != LogLevel::OFF) {
            EXPECT_TRUE(should_log(level));
        } else {
            EXPECT_FALSE(should_log(level));
        }
    }
}

TEST_F(TestRootLogger, create_logger)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, LogContentBufferSink>(name);
    EXPECT_NE(logger, nullptr);
    EXPECT_EQ(logger->name(), name);
    EXPECT_EQ(logger->sinks().size(), 1);
    auto *sink = reinterpret_cast<LogContentBufferSink *>(logger->sinks()[0].get());
    EXPECT_EQ(sink->buffer().size(), 0);
}

TEST_F(TestRootLogger, set_root_logger)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, LogContentBufferSink>(name);
    set_root_logger(logger);
    EXPECT_EQ(root_logger()->name(), name);
    EXPECT_EQ(root_logger_raw()->name(), name);
}

TEST_F(TestRootLogger, get_root_logger)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, LogContentBufferSink>(name);
    set_root_logger(logger);
    EXPECT_EQ(root_logger()->name(), name);
    EXPECT_EQ(root_logger_raw()->name(), name);

    root_logger()->set_level(LogLevel::INFO);
    EXPECT_EQ(root_logger()->level(), LogLevel::INFO);
    EXPECT_EQ(root_logger_raw()->level(), LogLevel::INFO);
    EXPECT_TRUE(should_log(LogLevel::INFO));
    EXPECT_FALSE(should_log(LogLevel::DEBUG));
}

TEST_F(TestRootLogger, log)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, LogContentBufferSink>(name);
    set_root_logger(logger);
    root_logger()->set_level(LogLevel::TRACE);
    trace(LOG_SRC_LOCAL, "trace {}", name);
    debug(LOG_SRC_LOCAL, "debug {}", name);
    info(LOG_SRC_LOCAL, "info {}", name);
    warn(LOG_SRC_LOCAL, "warn {}", name);
    error(LOG_SRC_LOCAL, "error {}", name);
    fatal(LOG_SRC_LOCAL, "fatal {}", name);

    trace("trace {}", name);
    debug("debug {}", name);
    info("info {}", name);
    warn("warn {}", name);
    error("error {}", name);
    fatal("fatal {}", name);

    trace(LOG_SRC_LOCAL, name);
    debug(LOG_SRC_LOCAL, name);
    info(LOG_SRC_LOCAL, name);
    warn(LOG_SRC_LOCAL, name);
    error(LOG_SRC_LOCAL, name);
    fatal(LOG_SRC_LOCAL, name);

    trace(name);
    debug(name);
    info(name);
    warn(name);
    error(name);
    fatal(name);
}

TEST_F(TestRootLogger, flush)
{
    const std::string name = get_logger_name(test_info_);
    auto sink = create_sink<LogContentBufferSink>();
    auto logger = create_logger<SyncLogger>(name, sink);
    set_root_logger(logger);

    root_logger()->set_level(LogLevel::TRACE);

    constexpr uint32_t MAX_ITEM_CNT = 100;
    for (uint32_t i = 0; i < MAX_ITEM_CNT; ++i) {
        root_logger()->log(LOG_SRC_LOCAL, sink->level(), i);
        EXPECT_EQ(sink->buffer().size(), i + 1);
        EXPECT_EQ(sink->disk().size(), 0);
    }
    root_logger()->flush();
    EXPECT_EQ(sink->buffer().size(), 0);
    EXPECT_EQ(sink->disk().size(), MAX_ITEM_CNT);
}

TEST_F(TestRootLogger, flush_on)
{
    const std::string name = get_logger_name(test_info_);
    auto sink = create_sink<LogContentBufferSink>();
    sink->set_level(LogLevel::TRACE);
    auto logger = create_logger<SyncLogger>(name, sink);
    set_root_logger(logger);
    root_logger()->set_level(LogLevel::TRACE);
    for (auto flushLevel : LOG_LEVELS) {
        // 设置刷新等级
        root_logger()->flush_on(flushLevel);
        for (uint32_t i = 0; i < LOG_LEVELS.size(); ++i) {
            LogLevel level = LOG_LEVELS[i];
            if (level == LogLevel::OFF) {
                break;
            }
            root_logger()->log(LOG_SRC_LOCAL, level, i);
            if (root_logger()->flush_level() == LogLevel::OFF ||
                level < root_logger()->flush_level()) {
                EXPECT_EQ(sink->buffer().size(), i + 1);
                EXPECT_EQ(sink->disk().size(), 0);
            } else {
                EXPECT_EQ(sink->buffer().size(), 0);
                EXPECT_EQ(sink->disk().size(), i + 1);
            }
        }
        sink->clear();
    }
}

TEST_F(TestRootLogger, set_pattern)
{
    const std::string name = get_logger_name(test_info_);
    auto sink = create_sink<LogContentBufferSink>();
    auto logger = create_logger<SyncLogger>(name, sink);
    set_root_logger(logger);
    root_logger()->set_pattern("%v");
    for (uint32_t i = 0; i < 100; i++) {
        root_logger()->error(i);
        EXPECT_EQ(std::to_string(i), sink->buffer()[i]);
    }
}

TEST_F(TestRootLogger, set_formatter)
{
    const std::string name = get_logger_name(test_info_);
    auto sink = create_sink<LogContentBufferSink>();
    auto logger = create_logger<SyncLogger>(name, sink);
    set_root_logger(logger);
    auto formatter = create_formatter<PatternFormatter>("%v");
    root_logger()->set_formatter(formatter);
    for (uint32_t i = 0; i < 100; i++) {
        root_logger()->error(i);
        EXPECT_EQ(std::to_string(i), sink->buffer()[i]);
    }
}

}  // namespace logging_test
