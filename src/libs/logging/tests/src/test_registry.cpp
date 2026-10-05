#include <cstdint>
#include <memory>
#include <string>

#include "detail/common.hpp"
#include "gtest/gtest.h"
#include "logging/loggers/sync_logger.hpp"
#include "logging/logging.hpp"
#include "logging/sinks/stdout_sink.hpp"

namespace logging_test {
using namespace origin::logging;

class TestRegistry : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override;
};

void TestRegistry::TearDown()
{
    shutdown();
}

TEST_F(TestRegistry, register_logger)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_TRUE(register_logger(logger));
    EXPECT_TRUE(is_logger_exists(name));
}

TEST_F(TestRegistry, register_logger_failed_when_logger_is_nullptr)
{
    EXPECT_FALSE(register_logger(nullptr));
}

TEST_F(TestRegistry, register_logger_failed_when_logger_already_exists)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_TRUE(register_logger(logger));
    auto logger2 = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_FALSE(register_logger(logger2));
}

TEST_F(TestRegistry, get_logger)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_TRUE(register_logger(logger));
    auto logger2 = get_logger(name);
    EXPECT_EQ(logger, logger2);
}

TEST_F(TestRegistry, get_logger_when_logger_not_exist)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = get_logger(name);
    EXPECT_EQ(nullptr, logger);
}

TEST_F(TestRegistry, remove_logger_when_logger_exist)
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

TEST_F(TestRegistry, remove_logger_when_logger_is_root_logger)
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

TEST_F(TestRegistry, remove_logger_when_logger_not_exist)
{
    const std::string name = get_logger_name(test_info_);
    EXPECT_FALSE(is_logger_exists(name));
    remove_logger(name);
    auto logger = get_logger(name);
    EXPECT_EQ(nullptr, logger);
    EXPECT_FALSE(is_logger_exists(name));
}

TEST_F(TestRegistry, is_logger_exist)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    EXPECT_FALSE(is_logger_exists(name));
    EXPECT_TRUE(register_logger(logger));
    EXPECT_TRUE(is_logger_exists(name));
}

TEST_F(TestRegistry, is_logger_exist_when_logger_not_exist)
{
    const std::string name = get_logger_name(test_info_);
    EXPECT_FALSE(is_logger_exists(name));
}

TEST_F(TestRegistry, register_or_replace_logger)
{
    const std::string name = get_logger_name(test_info_);

    auto logger = create_logger<SyncLogger, StdoutSink>(name);
    register_or_replace_logger(logger);

    auto logger2 = create_logger<SyncLogger, StdoutSink>(name);
    register_or_replace_logger(logger2);

    auto logger3 = get_logger(name);
    EXPECT_EQ(logger2, logger3);
}

TEST_F(TestRegistry, remove_all_loggers)
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

TEST_F(TestRegistry, set_root_task_pool)
{
    constexpr uint32_t capacity = 10;
    constexpr uint32_t threadCnt = 1;
    auto taskPool = create_task_pool(capacity, threadCnt);
    EXPECT_TRUE(set_root_task_pool(taskPool));
    auto taskPool1 = root_task_pool();
    EXPECT_EQ(taskPool, taskPool1);
}

TEST_F(TestRegistry, set_root_task_pool_failed_when_task_pool_is_nullptr)
{
    EXPECT_FALSE(set_root_task_pool(nullptr));
    auto taskPool1 = root_task_pool();
    EXPECT_EQ(nullptr, taskPool1);
}

TEST_F(TestRegistry, set_root_task_pool_failed_when_task_pool_already_exists)
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

TEST_F(TestRegistry, get_root_task_pool_failed_when_not_set)
{
    auto taskPool = root_task_pool();
    EXPECT_EQ(nullptr, taskPool);
}

}  // namespace logging_test
