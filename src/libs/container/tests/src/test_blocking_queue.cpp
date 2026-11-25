#include "gtest/gtest.h"

#include <cstdint>
#include <stdexcept>

#include "origin/container/blocking_queue.hpp"

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

// 验证队列支持无默认构造、仅可移动的元素类型。
class NoDefaultEntry {
public:
    NoDefaultEntry() = delete;
    explicit NoDefaultEntry(int32_t n) : x(n) {}
    NoDefaultEntry(const NoDefaultEntry&) = delete;
    NoDefaultEntry& operator=(const NoDefaultEntry&) = delete;
    NoDefaultEntry(NoDefaultEntry&&) = default;
    NoDefaultEntry& operator=(NoDefaultEntry&&) = default;
    ~NoDefaultEntry() = default;
    [[nodiscard]] int32_t get() const { return x; }

private:
    int32_t x = 0;
};

// 统计存活对象，验证入队/出队/clear/overrun 不会泄漏，也不会二次构造。
class LifecycleEntry {
public:
    explicit LifecycleEntry(int32_t n) : x(n) { ++alive; }
    LifecycleEntry(const LifecycleEntry&) = delete;
    LifecycleEntry& operator=(const LifecycleEntry&) = delete;
    LifecycleEntry(LifecycleEntry&& other) noexcept : x(other.x) { ++alive; }
    LifecycleEntry& operator=(LifecycleEntry&& other) noexcept
    {
        x = other.x;
        return *this;
    }
    ~LifecycleEntry() { --alive; }
    [[nodiscard]] int32_t get() const { return x; }

    static inline int32_t alive = 0;

private:
    int32_t x = 0;
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
        EXPECT_THROW(static_cast<void>(queue.at(i + 1)), std::out_of_range);
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

TEST_F(TestBlockingQueue, non_default_constructible)
{
    BlockingQueue<NoDefaultEntry> queue(2);
    EXPECT_TRUE(queue.emplace_back(42));
    EXPECT_EQ(queue.size(), 1);

    NoDefaultEntry item(0);
    EXPECT_TRUE(queue.dequeue(item));
    EXPECT_EQ(item.get(), 42);
    EXPECT_TRUE(queue.empty());
}

TEST_F(TestBlockingQueue, lifecycle_with_clear_and_overrun)
{
    LifecycleEntry::alive = 0;
    {
        BlockingQueue<LifecycleEntry> queue(2);
        EXPECT_TRUE(queue.emplace_back(1));
        EXPECT_TRUE(queue.emplace_back(2));
        EXPECT_EQ(LifecycleEntry::alive, 2);

        queue.emplace_back_overrun(3);
        EXPECT_EQ(queue.size(), 2);
        EXPECT_EQ(LifecycleEntry::alive, 2);

        queue.clear();
        EXPECT_EQ(LifecycleEntry::alive, 0);
        EXPECT_TRUE(queue.empty());

        EXPECT_TRUE(queue.emplace_back(4));
        EXPECT_EQ(LifecycleEntry::alive, 1);
    }
    EXPECT_EQ(LifecycleEntry::alive, 0);
}

}  // namespace test::test_container::test_blocking_queue
