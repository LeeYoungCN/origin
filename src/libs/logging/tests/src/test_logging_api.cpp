#include <cstdint>
#include <memory>
#include <string>

#include "detail/common.hpp"
#include "detail/mock_sinks/log_content_buffer_sink.hpp"
#include "gtest/gtest.h"
#include "origin/logging/loggers/sync_logger.hpp"
#include "origin/logging/logging.hpp"
#include "origin/logging/sinks/stdout_sink.hpp"

namespace logging_test {
using namespace origin::logging;

class TestLoggingApi : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override;
};

void TestLoggingApi::TearDown()
{
    shutdown();
}

TEST_F(TestLoggingApi, register_logger)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_TRUE(register_logger(logger));
    EXPECT_TRUE(is_logger_exists(name));
}

TEST_F(TestLoggingApi, register_logger_failed_when_logger_is_nullptr)
{
    EXPECT_FALSE(register_logger(nullptr));
}

TEST_F(TestLoggingApi, register_logger_failed_when_logger_already_exists)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_TRUE(register_logger(logger));
    auto logger2 = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_FALSE(register_logger(logger2));
}

TEST_F(TestLoggingApi, get_logger)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_TRUE(register_logger(logger));
    auto logger2 = get_logger(name);
    EXPECT_EQ(logger, logger2);
}

TEST_F(TestLoggingApi, get_logger_when_logger_not_exist)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = get_logger(name);
    EXPECT_EQ(nullptr, logger);
}

TEST_F(TestLoggingApi, remove_logger_when_logger_exist)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_TRUE(register_logger(logger));

    auto logger2 = get_logger(name);
    EXPECT_EQ(logger, logger2);

    remove_logger(name);
    auto logger3 = get_logger(name);
    EXPECT_EQ(nullptr, logger3);
    EXPECT_FALSE(is_logger_exists(name));
}

TEST_F(TestLoggingApi, remove_logger_when_logger_is_root_logger)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    set_root_logger(logger);

    auto logger2 = get_logger(name);
    EXPECT_EQ(logger, logger2);

    remove_logger(name);
    EXPECT_EQ(nullptr, get_logger(name));
    EXPECT_EQ(nullptr, root_logger());
    EXPECT_EQ(nullptr, root_logger_raw());
    EXPECT_FALSE(is_logger_exists(name));
}

TEST_F(TestLoggingApi, remove_logger_when_logger_not_exist)
{
    const std::string name = get_logger_name(test_info_);
    EXPECT_FALSE(is_logger_exists(name));
    remove_logger(name);
    auto logger = get_logger(name);
    EXPECT_EQ(nullptr, logger);
    EXPECT_FALSE(is_logger_exists(name));
}

TEST_F(TestLoggingApi, is_logger_exist)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_FALSE(is_logger_exists(name));
    EXPECT_TRUE(register_logger(logger));
    EXPECT_TRUE(is_logger_exists(name));
}

TEST_F(TestLoggingApi, is_logger_exist_when_logger_not_exist)
{
    const std::string name = get_logger_name(test_info_);
    EXPECT_FALSE(is_logger_exists(name));
}

TEST_F(TestLoggingApi, register_or_replace_logger)
{
    const std::string name = get_logger_name(test_info_);

    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    register_or_replace_logger(logger);

    auto logger2 = create_logger<SyncLogger, StdoutSink>(name);
    register_or_replace_logger(logger2);

    auto logger3 = get_logger(name);
    EXPECT_EQ(logger2, logger3);
}

TEST_F(TestLoggingApi, remove_all_loggers)
{
    const std::string name1 = get_logger_name(test_info_);
    auto logger1 = create_logger<SyncLogger, StdoutSink>(name1);
    EXPECT_TRUE(register_logger(logger1));

    const std::string name2 = name1 + "_2";
    auto logger2 = create_logger<SyncLogger, StdoutSink>(name2);
    EXPECT_TRUE(register_logger(logger2));

    EXPECT_TRUE(is_logger_exists(name1));
    EXPECT_TRUE(is_logger_exists(name2));

    remove_all();

    EXPECT_FALSE(is_logger_exists(name1));
    EXPECT_FALSE(is_logger_exists(name2));
}

TEST_F(TestLoggingApi, set_root_task_pool)
{
    constexpr uint32_t capacity = 10;
    constexpr uint32_t threadCnt = 1;
    auto taskPool = create_task_pool(capacity, threadCnt);
    EXPECT_TRUE(set_root_task_pool(taskPool));
    auto taskPool1 = root_task_pool();
    EXPECT_EQ(taskPool, taskPool1);
}

TEST_F(TestLoggingApi, set_root_task_pool_failed_when_task_pool_is_nullptr)
{
    EXPECT_FALSE(set_root_task_pool(nullptr));
    auto taskPool1 = root_task_pool();
    EXPECT_EQ(nullptr, taskPool1);
}

TEST_F(TestLoggingApi, set_root_task_pool_failed_when_task_pool_already_exists)
{
    constexpr uint32_t capacity = 10;
    constexpr uint32_t threadCnt = 1;
    auto taskPool = create_task_pool(capacity, threadCnt);
    EXPECT_TRUE(set_root_task_pool(taskPool));

    auto taskPool1 = create_task_pool(capacity, threadCnt);
    EXPECT_FALSE(set_root_task_pool(taskPool1));

    auto taskPool2 = root_task_pool();
    EXPECT_EQ(taskPool2, taskPool);
}

TEST_F(TestLoggingApi, get_root_task_pool_failed_when_not_set)
{
    auto taskPool = root_task_pool();
    EXPECT_EQ(nullptr, taskPool);
}

TEST_F(TestLoggingApi, log_all_writes_to_every_registered_logger)
{
    remove_all();  // 去掉进程启动时默认创建的 root logger，保证计数确定
    const std::string name1 = get_logger_name(test_info_);
    const std::string name2 = name1 + "_2";

    auto sink1 = std::make_shared<LogContentBufferSink>();
    auto sink2 = std::make_shared<LogContentBufferSink>();
    sink1->set_level(LogLevel::TRACE);
    sink2->set_level(LogLevel::TRACE);
    sink1->set_pattern("%n=%v");
    sink2->set_pattern("%n=%v");

    auto logger1 = create_logger<SyncLogger>(name1, sink1);
    auto logger2 = create_logger<SyncLogger>(name2, sink2);
    logger1->set_level(LogLevel::TRACE);
    logger2->set_level(LogLevel::TRACE);
    ASSERT_TRUE(register_logger(logger1));
    ASSERT_TRUE(register_logger(logger2));

    const uint32_t count = log_all(LOG_SRC_LOCAL, LogLevel::INFO, "hello {}", 1);

    EXPECT_EQ(count, 2u);
    ASSERT_EQ(sink1->buffer().size(), 1u);
    ASSERT_EQ(sink2->buffer().size(), 1u);
    EXPECT_EQ(sink1->buffer()[0], name1 + "=hello 1");
    EXPECT_EQ(sink2->buffer()[0], name2 + "=hello 1");
}

TEST_F(TestLoggingApi, log_all_skips_logger_below_level)
{
    remove_all();
    const std::string name1 = get_logger_name(test_info_);
    const std::string name2 = name1 + "_2";

    auto sink1 = std::make_shared<LogContentBufferSink>();
    auto sink2 = std::make_shared<LogContentBufferSink>();
    sink1->set_level(LogLevel::TRACE);
    sink2->set_level(LogLevel::TRACE);
    sink1->set_pattern("%v");
    sink2->set_pattern("%v");

    auto logger1 = create_logger<SyncLogger>(name1, sink1);
    auto logger2 = create_logger<SyncLogger>(name2, sink2);
    logger1->set_level(LogLevel::TRACE);
    logger2->set_level(LogLevel::ERR);
    ASSERT_TRUE(register_logger(logger1));
    ASSERT_TRUE(register_logger(logger2));

    const uint32_t count = log_all(LOG_SRC_LOCAL, LogLevel::INFO, "filtered");

    EXPECT_EQ(count, 1u);
    EXPECT_EQ(sink1->buffer().size(), 1u);
    EXPECT_EQ(sink2->buffer().size(), 0u);
}

TEST_F(TestLoggingApi, log_all_returns_zero_when_registry_empty)
{
    remove_all();
    EXPECT_EQ(log_all(LOG_SRC_LOCAL, LogLevel::INFO, "nobody"), 0u);
}

TEST_F(TestLoggingApi, log_all_accepts_plain_string_without_recursion)
{
    remove_all();
    auto sink = std::make_shared<LogContentBufferSink>();
    sink->set_level(LogLevel::TRACE);
    sink->set_pattern("%v");

    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger>(name, sink);
    logger->set_level(LogLevel::TRACE);
    ASSERT_TRUE(register_logger(logger));

    // 走 const T& 重载：内部必须先转成 string_view 再委托，否则会递归到栈溢出
    const std::string text = "plain string";
    EXPECT_EQ(log_all(LOG_SRC_LOCAL, LogLevel::INFO, text), 1u);

    ASSERT_EQ(sink->buffer().size(), 1u);
    EXPECT_EQ(sink->buffer()[0], text);
}

TEST_F(TestLoggingApi, log_all_accepts_numeric_and_string_view_message)
{
    remove_all();
    auto sink = std::make_shared<LogContentBufferSink>();
    sink->set_level(LogLevel::TRACE);
    sink->set_pattern("%v");

    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger>(name, sink);
    logger->set_level(LogLevel::TRACE);
    ASSERT_TRUE(register_logger(logger));

    EXPECT_EQ(log_all(LOG_SRC_LOCAL, LogLevel::INFO, 42), 1u);
    ASSERT_EQ(sink->buffer().size(), 1u);
    EXPECT_EQ(sink->buffer()[0], "42");

    sink->clear();
    const std::string_view text = "from string_view";
    EXPECT_EQ(log_all(LOG_SRC_LOCAL, LogLevel::INFO, text), 1u);
    ASSERT_EQ(sink->buffer().size(), 1u);
    EXPECT_EQ(sink->buffer()[0], text);
}

TEST_F(TestLoggingApi, log_all_treats_bare_literal_as_message)
{
    remove_all();
    auto sink = std::make_shared<LogContentBufferSink>();
    sink->set_level(LogLevel::TRACE);
    sink->set_pattern("%v");

    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger>(name, sink);
    logger->set_level(LogLevel::TRACE);
    ASSERT_TRUE(register_logger(logger));

    // 不带参数的字面量走 string_view 重载，不会被当成格式串解析
    EXPECT_EQ(log_all(LOG_SRC_LOCAL, LogLevel::INFO, "{plain}"), 1u);
    ASSERT_EQ(sink->buffer().size(), 1u);
    EXPECT_EQ(sink->buffer()[0], "{plain}");
}

TEST_F(TestLoggingApi, set_level_all_applies_to_all_loggers)
{
    remove_all();
    const std::string name1 = get_logger_name(test_info_);
    const std::string name2 = name1 + "_2";
    auto logger1 = create_logger<SyncLogger, StdoutSink>(name1);
    auto logger2 = create_logger<SyncLogger, StdoutSink>(name2);
    ASSERT_TRUE(register_logger(logger1));
    ASSERT_TRUE(register_logger(logger2));

    set_level_all(LogLevel::WARN);

    EXPECT_EQ(logger1->level(), LogLevel::WARN);
    EXPECT_EQ(logger2->level(), LogLevel::WARN);

    // 之后经 initialize_logger 注册的 logger 继承全局级别
    auto logger3 = create_logger<SyncLogger, StdoutSink>(name1 + "_3");
    initialize_logger(logger3);
    EXPECT_EQ(logger3->level(), LogLevel::WARN);
}

TEST_F(TestLoggingApi, flush_on_all_applies_to_all_loggers)
{
    remove_all();
    const std::string name1 = get_logger_name(test_info_);
    const std::string name2 = name1 + "_2";
    auto logger1 = create_logger<SyncLogger, StdoutSink>(name1);
    auto logger2 = create_logger<SyncLogger, StdoutSink>(name2);
    ASSERT_TRUE(register_logger(logger1));
    ASSERT_TRUE(register_logger(logger2));

    flush_on_all(LogLevel::ERR);

    EXPECT_EQ(logger1->flush_level(), LogLevel::ERR);
    EXPECT_EQ(logger2->flush_level(), LogLevel::ERR);
}

TEST_F(TestLoggingApi, flush_all_flushes_every_sink)
{
    remove_all();
    auto sink = std::make_shared<LogContentBufferSink>();
    sink->set_level(LogLevel::TRACE);
    sink->set_pattern("%v");

    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger>(name, sink);
    logger->set_level(LogLevel::TRACE);
    ASSERT_TRUE(register_logger(logger));

    logger->info("hello");
    ASSERT_EQ(sink->buffer().size(), 1u);
    EXPECT_EQ(sink->disk().size(), 0u);

    flush_all();

    EXPECT_EQ(sink->buffer().size(), 0u);
    EXPECT_EQ(sink->disk().size(), 1u);
}

TEST_F(TestLoggingApi, set_pattern_all_applies_to_all_loggers)
{
    remove_all();
    auto sink = std::make_shared<LogContentBufferSink>();
    sink->set_level(LogLevel::TRACE);

    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger>(name, sink);
    logger->set_level(LogLevel::TRACE);
    ASSERT_TRUE(register_logger(logger));

    set_pattern_all("%v");
    logger->info("hello");

    ASSERT_EQ(sink->buffer().size(), 1u);
    EXPECT_EQ(sink->buffer()[0], "hello");  // 没有前缀，说明 pattern 生效
}

}  // namespace logging_test
