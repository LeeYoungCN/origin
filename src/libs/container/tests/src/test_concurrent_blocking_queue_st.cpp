#include <cstdint>
#include <stdexcept>

#include "container/concurrent_blocking_queue.hpp"
#include "common/types/thread_types.h"
#include "gtest/gtest.h"
#include "utils/thread_utils.h"

namespace test::test_container::test_concurrent_queue {
using namespace origin::container;

class TestConcurrentBlockingQueueSt : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};
};

TEST_F(TestConcurrentBlockingQueueSt, create_default)
{
    auto q = ConcurrentBlockingQueue<uint32_t>();
    EXPECT_EQ(q.size(), 0);
    EXPECT_EQ(q.capacity(), CONCURRENT_BLOCKING_QUEUE_DEFAULT_CAPACITY);
    EXPECT_TRUE(q.empty());
}

TEST_F(TestConcurrentBlockingQueueSt, create_capacity)
{
    auto q = ConcurrentBlockingQueue<uint32_t>(1024);
    EXPECT_EQ(q.size(), 0);
    EXPECT_EQ(q.capacity(), 1024);
    EXPECT_TRUE(q.empty());
}

TEST_F(TestConcurrentBlockingQueueSt, create_capacity_invalid)
{
    EXPECT_THROW(ConcurrentBlockingQueue<uint32_t>(0), std::invalid_argument);
}

TEST_F(TestConcurrentBlockingQueueSt, out_of_range)
{
    uint32_t capacity = 100;
    auto queue = ConcurrentBlockingQueue<uint32_t>(capacity);
    EXPECT_EQ(capacity, queue.capacity());
    EXPECT_TRUE(queue.empty());

    for (uint32_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(queue.enqueue(i)) << "i = " << i;
        EXPECT_EQ(i + 1, queue.size());
        EXPECT_THROW(queue.at(i + 1), std::out_of_range);
    }
}

TEST_F(TestConcurrentBlockingQueueSt, enqueue)
{
    uint32_t capacity = 100;
    auto queue = ConcurrentBlockingQueue<uint32_t>(capacity);

    EXPECT_EQ(capacity, queue.capacity());
    EXPECT_TRUE(queue.empty());

    for (uint32_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(queue.enqueue(i)) << "i = " << i;
        EXPECT_EQ(i + 1, queue.size());
        EXPECT_EQ(queue[i], i);
    }

    EXPECT_FALSE(queue.enqueue(0));
    EXPECT_TRUE(queue.full());
}

TEST_F(TestConcurrentBlockingQueueSt, enqueue_overrun)
{
    uint32_t capacity = 100;
    auto queue = ConcurrentBlockingQueue<uint32_t>(capacity);

    EXPECT_EQ(capacity, queue.capacity());
    EXPECT_TRUE(queue.empty());

    for (uint32_t i = 0; i < 2 * capacity; i++) {
        queue.enqueue_overrun(i);
        if (i < capacity) {
            EXPECT_EQ(i + 1, queue.size());
        } else {
            EXPECT_EQ(capacity, queue.size());
            EXPECT_EQ(queue[0], i - capacity + 1);
        }
    }

    for (uint32_t i = 0; i < capacity; ++i) {
        EXPECT_EQ(queue.at(i), capacity + i);
    }
}

TEST_F(TestConcurrentBlockingQueueSt, enqueue_discard_counter)
{
    uint32_t capacity = 100;
    auto queue = ConcurrentBlockingQueue<uint32_t>(capacity);

    EXPECT_EQ(capacity, queue.capacity());
    EXPECT_TRUE(queue.empty());

    for (uint32_t i = 0; i < 2 * capacity; i++) {
        auto rst = queue.enqueue(i);
        if (i < capacity) {
            EXPECT_TRUE(rst);
            EXPECT_EQ(i + 1, queue.size());
        } else {
            EXPECT_EQ(capacity, queue.size());
            EXPECT_FALSE(rst);
            EXPECT_EQ(i - capacity + 1, queue.discard_counter());
        }
    }
}

TEST_F(TestConcurrentBlockingQueueSt, dequeue)
{
    uint32_t capacity = 100;
    auto queue = ConcurrentBlockingQueue<uint32_t>(capacity);

    for (uint32_t i = 0; i < capacity; i++) {
        queue.enqueue(i);
    }

    for (uint32_t i = 0; i < 2 * capacity; i++) {
        uint32_t n = 0;
        if (i < capacity) {
            EXPECT_TRUE(queue.dequeue(n));
            EXPECT_EQ(n, i);
            EXPECT_EQ(capacity - i - 1, queue.size());
        } else {
            EXPECT_FALSE(queue.dequeue(n));
            EXPECT_EQ(n, 0);
            EXPECT_TRUE(queue.empty());
            EXPECT_EQ(queue.size(), 0);
        }
    }
}

TEST_F(TestConcurrentBlockingQueueSt, queue_and_dequeue_wait_for)
{
    uint32_t capacity = CONCURRENT_BLOCKING_QUEUE_DEFAULT_CAPACITY;
    auto queue = ConcurrentBlockingQueue<uint32_t>(capacity);

    for (uint32_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(queue.enqueue_wait_for(i));
    }

    uint32_t lastNum = 0;
    for (uint32_t i = 0; i < capacity / 2; i++) {
        EXPECT_TRUE(queue.dequeue_wait_for(lastNum));
    }

    for (uint32_t i = 0; i < capacity / 2; i++) {
        EXPECT_TRUE(queue.enqueue_wait_for(capacity + i));
    }

    for (uint32_t i = 0; i < capacity; i++) {
        EXPECT_EQ(queue[i], capacity / 2 + i);
    }
}
}  // namespace test::test_container::test_concurrent_queue
