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

#include "gtest/gtest.h"
#include "utils/thread_utils.h"
using namespace origin::thread;
TEST(TestThreadUtils, ThreadId)
{
    EXPECT_NE(get_curr_thread_id(), 0);
}

TEST(TestThreadUtils, ThreadName)
{
    EXPECT_STREQ(get_curr_thread_name(), "");
    set_curr_thread_name("main");
    EXPECT_STREQ(get_curr_thread_name(), "main");
}
