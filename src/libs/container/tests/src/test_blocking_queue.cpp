#include <cstdint>
#include <stdexcept>

#include "container/blocking_queue.hpp"
#include "gtest/gtest.h"

namespace test::test_container::test_blocking_queue {
using namespace origin::container;

class TestEmplaceEntry {
public:
    TestEmplaceEntry() = default;
    ~TestEmplaceEntry() = default;
    explicit TestEmplaceEntry(int32_t n) : x(n) {}
    [[nodiscard]] int32_t get() const { return x; };

private:
    int32_t x = -1;
};

class TestBlockingQueue : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};
};

TEST_F(TestBlockingQueue, create_default)
{
    auto q = BlockingQueue<uint32_t>();
    EXPECT_EQ(q.size(), 0);
    EXPECT_EQ(q.capacity(), BLOCKING_QUEUE_DEFAULT_CAPACITY);
    EXPECT_TRUE(q.empty());
}

TEST_F(TestBlockingQueue, create_capacity)
{
    auto q = BlockingQueue<uint32_t>(1024);
    EXPECT_EQ(q.size(), 0);
    EXPECT_EQ(q.capacity(), 1024);
    EXPECT_TRUE(q.empty());
}

TEST_F(TestBlockingQueue, create_capacity_invalid)
{
    EXPECT_THROW(BlockingQueue<uint32_t>(0), std::invalid_argument);
}

TEST_F(TestBlockingQueue, out_of_range)
{
    uint32_t capacity = 100;
    auto queue = BlockingQueue<uint32_t>(capacity);
    EXPECT_EQ(capacity, queue.capacity());
    EXPECT_TRUE(queue.empty());

    for (uint32_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(queue.enqueue(i)) << "i = " << i;
        EXPECT_EQ(i + 1, queue.size());
        EXPECT_THROW(queue.at(i + 1), std::out_of_range);
    }
}

TEST_F(TestBlockingQueue, enqueue)
{
    uint32_t capacity = 100;
    auto queue = BlockingQueue<uint32_t>(capacity);

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

TEST_F(TestBlockingQueue, emplace_back)
{
    uint32_t capacity = 100;
    auto queue = BlockingQueue<TestEmplaceEntry>(capacity);

    EXPECT_EQ(capacity, queue.capacity());
    EXPECT_TRUE(queue.empty());

    for (uint32_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(queue.emplace_back(i)) << "i = " << i;
        EXPECT_EQ(i + 1, queue.size());
        EXPECT_EQ(queue[i].get(), i);
    }

    EXPECT_FALSE(queue.emplace_back(0));
    EXPECT_TRUE(queue.full());
}

TEST_F(TestBlockingQueue, clear)
{
    uint32_t capacity = 100;
    auto queue = BlockingQueue<uint32_t>(capacity);

    EXPECT_EQ(capacity, queue.capacity());
    EXPECT_TRUE(queue.empty());

    for (uint32_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(queue.enqueue(i)) << "i = " << i;
        EXPECT_EQ(i + 1, queue.size());
        EXPECT_EQ(queue[i], i);
    }

    EXPECT_TRUE(queue.full());
    queue.clear();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(TestBlockingQueue, enqueue_overrun)
{
    uint32_t capacity = 100;
    auto queue = BlockingQueue<uint32_t>(capacity);

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

TEST_F(TestBlockingQueue, emplace_back_overrun)
{
    uint32_t capacity = 100;
    auto queue = BlockingQueue<TestEmplaceEntry>(capacity);

    EXPECT_EQ(capacity, queue.capacity());
    EXPECT_TRUE(queue.empty());

    for (uint32_t i = 0; i < 2 * capacity; i++) {
        queue.emplace_back_overrun(i);
        if (i < capacity) {
            EXPECT_EQ(i + 1, queue.size());
        } else {
            EXPECT_EQ(capacity, queue.size());
            EXPECT_EQ(queue[0].get(), i - capacity + 1);
        }
    }

    for (uint32_t i = 0; i < capacity; ++i) {
        EXPECT_EQ(queue.at(i).get(), capacity + i);
    }
}

TEST_F(TestBlockingQueue, dequeue)
{
    uint32_t capacity = 100;
    auto queue = BlockingQueue<uint32_t>(capacity);

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

TEST_F(TestBlockingQueue, queue_and_dequeue)
{
    uint32_t capacity = 100;
    auto queue = BlockingQueue<uint32_t>(capacity);

    for (uint32_t i = 0; i < capacity; i++) {
        EXPECT_TRUE(queue.enqueue(i));
    }

    uint32_t lastNum = 0;
    for (uint32_t i = 0; i < capacity / 2; i++) {
        EXPECT_TRUE(queue.dequeue(lastNum));
    }

    for (uint32_t i = 0; i < capacity / 2; i++) {
        EXPECT_TRUE(queue.enqueue(capacity + i));
    }

    for (uint32_t i = 0; i < capacity; i++) {
        EXPECT_EQ(queue[i], capacity / 2 + i);
    }
}

}  // namespace test::test_container::test_blocking_queue
