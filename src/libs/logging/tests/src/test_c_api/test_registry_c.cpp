#include <cstdint>
#include <string>

#include "detail/common.hpp"
#include "gtest/gtest.h"
#include "logging/c/logging_c.h"

namespace logging_test {
using namespace origin::logging;

class TestRegistryC : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override;

    SinkSt* _sink = origin_create_stdout_sink();
};

void TestRegistryC::TearDown()
{
    origin_destroy_sink(_sink);
    origin_shutdown();
}

TEST_F(TestRegistryC, register_logger)
{
    const std::string name = get_logger_name(test_info_);

    auto logger = origin_create_sync_logger(name.c_str(), &_sink, 1);
    EXPECT_TRUE(origin_register_logger(logger));
    EXPECT_TRUE(origin_is_logger_exists(name.c_str()));
    origin_destroy_logger(logger);
}

TEST_F(TestRegistryC, register_logger_failed_when_logger_is_nullptr)
{
    EXPECT_FALSE(origin_register_logger(nullptr));

    auto logger = create_mock_logger_st(nullptr);
    EXPECT_FALSE(origin_register_logger(logger));
    destroy_mock_logger_st(logger);
}

TEST_F(TestRegistryC, register_logger_failed_when_logger_already_exists)
{
    const std::string name = get_logger_name(test_info_);

    auto logger = origin_create_sync_logger(name.c_str(), &_sink, 1);
    EXPECT_TRUE(origin_register_logger(logger));
    EXPECT_TRUE(origin_is_logger_exists(name.c_str()));

    auto logger1 = origin_create_sync_logger(name.c_str(), &_sink, 1);
    EXPECT_FALSE(origin_register_logger(logger1));

    origin_destroy_logger(logger);
    origin_destroy_logger(logger1);
}

TEST_F(TestRegistryC, get_logger)
{
    const std::string name = get_logger_name(test_info_);

    auto logger = origin_create_sync_logger(name.c_str(), &_sink, 1);
    EXPECT_TRUE(origin_register_logger(logger));

    auto logger1 = origin_get_logger(name.c_str());
    EXPECT_NE(logger1, nullptr);
    EXPECT_EQ(((MockLoggerSt*)logger)->ptr, ((MockLoggerSt*)logger1)->ptr);

    origin_destroy_logger(logger);
    origin_destroy_logger(logger1);
}

TEST_F(TestRegistryC, get_logger_when_logger_not_exist)
{
    const std::string name = get_logger_name(test_info_);

    EXPECT_EQ(nullptr, origin_get_logger(name.c_str()));
}

TEST_F(TestRegistryC, remove_logger_when_logger_exist)
{
    const std::string name = get_logger_name(test_info_);

    auto logger = origin_create_sync_logger(name.c_str(), &_sink, 1);
    EXPECT_TRUE(origin_register_logger(logger));

    auto logger1 = origin_get_logger(name.c_str());
    EXPECT_EQ(((MockLoggerSt*)logger)->ptr, ((MockLoggerSt*)logger1)->ptr);

    origin_remove_logger(name.c_str());

    EXPECT_EQ(nullptr, origin_get_logger(name.c_str()));
    EXPECT_FALSE(origin_is_logger_exists(name.c_str()));

    origin_destroy_logger(logger);
    origin_destroy_logger(logger1);
}

TEST_F(TestRegistryC, remove_logger_when_logger_is_root_logger)
{
    const std::string name = get_logger_name(test_info_);

    auto logger = origin_create_sync_logger(name.c_str(), &_sink, 1);
    EXPECT_TRUE(origin_register_logger(logger));
    origin_set_root_logger(logger);

    auto logger1 = origin_get_logger(name.c_str());
    EXPECT_EQ(((MockLoggerSt*)logger)->ptr, ((MockLoggerSt*)logger1)->ptr);

    origin_remove_logger(name.c_str());
    EXPECT_EQ(nullptr, origin_get_logger(name.c_str()));
    EXPECT_EQ(nullptr, origin_root_logger());
    EXPECT_FALSE(origin_is_logger_exists(name.c_str()));

    origin_destroy_logger(logger);
    origin_destroy_logger(logger1);
}

TEST_F(TestRegistryC, remove_logger_when_logger_not_exist)
{
    const std::string name = get_logger_name(test_info_);

    EXPECT_FALSE(origin_is_logger_exists(name.c_str()));
    origin_remove_logger(name.c_str());
    EXPECT_EQ(nullptr, origin_get_logger(name.c_str()));
    EXPECT_FALSE(origin_is_logger_exists(name.c_str()));
}

TEST_F(TestRegistryC, is_logger_exist)
{
    const std::string name = get_logger_name(test_info_);
    auto logger = origin_create_sync_logger(name.c_str(), &_sink, 1);
    EXPECT_TRUE(origin_register_logger(logger));
    EXPECT_TRUE(origin_is_logger_exists(name.c_str()));
    origin_destroy_logger(logger);
}

TEST_F(TestRegistryC, is_logger_exist_when_logger_not_exist)
{
    const std::string name = get_logger_name(test_info_);
    EXPECT_FALSE(origin_is_logger_exists(name.c_str()));
}

TEST_F(TestRegistryC, register_or_replace_logger)
{
    const std::string name = get_logger_name(test_info_);

    auto logger = origin_create_sync_logger(name.c_str(), &_sink, 1);
    origin_register_or_replace_logger(logger);

    auto logger1 = origin_create_sync_logger(name.c_str(), &_sink, 1);
    origin_register_or_replace_logger(logger1);

    auto logger2 = origin_get_logger(name.c_str());
    EXPECT_EQ(((MockLoggerSt*)logger1)->ptr, ((MockLoggerSt*)logger2)->ptr);
    origin_destroy_logger(logger);
    origin_destroy_logger(logger1);
    origin_destroy_logger(logger2);
}

TEST_F(TestRegistryC, remove_all_loggers)
{
    const std::string name = get_logger_name(test_info_);

    auto logger1 = origin_create_sync_logger(name.c_str(), &_sink, 1);
    EXPECT_TRUE(origin_register_logger(logger1));

    const std::string name2 = name + "_2";
    auto logger2 = origin_create_sync_logger(name2.c_str(), &_sink, 1);
    EXPECT_TRUE(origin_register_logger(logger2));

    EXPECT_TRUE(origin_is_logger_exists(name.c_str()));
    EXPECT_TRUE(origin_is_logger_exists(name2.c_str()));

    origin_remove_all();

    EXPECT_FALSE(origin_is_logger_exists(name.c_str()));
    EXPECT_FALSE(origin_is_logger_exists(name2.c_str()));

    origin_destroy_logger(logger1);
    origin_destroy_logger(logger2);
}

TEST_F(TestRegistryC, set_root_task_pool)
{
    constexpr uint32_t capacity = 10;
    constexpr uint32_t threadCnt = 1;
    auto taskPool = origin_create_task_pool(capacity, threadCnt);
    EXPECT_TRUE(origin_set_root_task_pool(taskPool));
    auto taskPool1 = origin_root_task_pool();
    EXPECT_EQ(((MockTaskPoolSt*)taskPool)->ptr, ((MockTaskPoolSt*)taskPool1)->ptr);

    origin_destroy_task_pool(taskPool);
    origin_destroy_task_pool(taskPool1);
}

TEST_F(TestRegistryC, set_root_task_pool_failed_when_task_pool_is_nullptr)
{
    EXPECT_FALSE(origin_set_root_task_pool(nullptr));
    EXPECT_EQ(nullptr, origin_root_task_pool());

    auto mockTaskPool = create_mock_task_pool_st(nullptr);
    EXPECT_FALSE(origin_set_root_task_pool(mockTaskPool));
    EXPECT_EQ(nullptr, origin_root_task_pool());
    destroy_mock_task_pool_st(mockTaskPool);
}

TEST_F(TestRegistryC, set_root_task_pool_failed_when_task_pool_already_exists)
{
    constexpr uint32_t capacity = 10;
    constexpr uint32_t threadCnt = 1;
    auto taskPool = origin_create_task_pool(capacity, threadCnt);
    EXPECT_TRUE(origin_set_root_task_pool(taskPool));

    auto taskPool1 = origin_create_task_pool(capacity, threadCnt);
    EXPECT_FALSE(origin_set_root_task_pool(taskPool1));

    auto taskPool2 = origin_root_task_pool();
    EXPECT_EQ(((MockTaskPoolSt*)taskPool2)->ptr, ((MockTaskPoolSt*)taskPool)->ptr);
    origin_destroy_task_pool(taskPool);
    origin_destroy_task_pool(taskPool1);
    origin_destroy_task_pool(taskPool2);
}

TEST_F(TestRegistryC, get_root_task_pool_failed_when_not_set)
{
    EXPECT_EQ(nullptr, origin_root_task_pool());
}

}  // namespace logging_test
