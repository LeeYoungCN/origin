/**
 * @file test_thread_utils.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-08-07
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "origin/common/common_error_code.h"

#include "origin/thread/thread.h"

#include "gtest/gtest.h"
using namespace origin::thread;
TEST(TestThreadUtils, ThreadId)
{
    EXPECT_NE(get_curr_thread_id(), 0);
}

TEST(TestThreadUtils, ThreadName)
{
    std::string name;
    ASSERT_EQ(get_curr_thread_name(name), ERR_COMM_SUCCESS);
    EXPECT_STREQ(name.c_str(), "");
    ASSERT_EQ(set_curr_thread_name("main"), ERR_COMM_SUCCESS);
    ASSERT_EQ(get_curr_thread_name(name), ERR_COMM_SUCCESS);
    EXPECT_STREQ(name.c_str(), "main");
}
