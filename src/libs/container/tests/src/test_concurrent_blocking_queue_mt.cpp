#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#include "container/concurrent_blocking_queue.hpp"
#include "common/debug/debug_logger.h"
#include "common/types/thread_types.h"
#include "gtest/gtest.h"
#include "utils/date_time_utils.h"
#include "utils/thread_utils.h"

namespace {
constexpr uint32_t MAX_NUM = 2048;
}

namespace test::test_container::test_concurrent_queue {
using namespace origin::container;
using namespace origin::date_time;
using namespace origin::thread;

struct TestEntry {
    uint32_t num;
    ThreadId tid;

    TestEntry() : num(0), tid(get_curr_thread_id()) {}
    explicit TestEntry(uint32_t n) : num(n), tid(get_curr_thread_id()) {}
};

class TestConcurrentBlockingQueueMt : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override;

protected:
    std::shared_ptr<ConcurrentBlockingQueue<TestEntry>> _queue;
    std::vector<std::thread> _threads;
    uint32_t _producerCnt = 0;
    std::atomic<bool> _isRunning = true;

    uint32_t _totalProduceItemCnt = 0;
    std::atomic<uint32_t> _totalConsumeItemCnt = 0;
    std::map<ThreadId, std::vector<TestEntry>> _itemMap;
    std::mutex _mapMtx;

protected:
    void ProducerLoop();
    void ConsumerLoopWait();
    void ConsumerLoopWaitFor();
    void StartThread(uint32_t producerCnt, uint32_t consumerCnt);
    void WaitFinish();
    void TestResult();
    void RunTest(uint32_t producerCnt, uint32_t consumerCnt,
                 uint32_t capacity = CONCURRENT_BLOCKING_QUEUE_DEFAULT_CAPACITY);
};

void TestConcurrentBlockingQueueMt::TearDown()
{
    for (auto& t : _threads) {
        t.join();
    }
}

void TestConcurrentBlockingQueueMt::ProducerLoop()
{
    ORIGIN_DEBUG_INFO("Producer thread start. curr: {}, max: {}.",
                      _totalConsumeItemCnt.load(),
                      _totalProduceItemCnt);
    for (uint32_t i = 0; i < MAX_NUM; ++i) {
        _queue->enqueue_wait(TestEntry(i));
    }
    ORIGIN_DEBUG_INFO("Producer thread finish. curr: {}, max: {}.",
                      _totalConsumeItemCnt.load(),
                      _totalProduceItemCnt);
}

void TestConcurrentBlockingQueueMt::ConsumerLoopWait()
{
    ORIGIN_DEBUG_INFO("Consumer thread start. Curr total consume: {}, max: {}.",
                      _totalConsumeItemCnt.load(),
                      _totalProduceItemCnt);
    uint32_t dequeueItemCnt = 0;
    while (_totalConsumeItemCnt < _totalProduceItemCnt) {
        std::lock_guard lock(_mapMtx);
        TestEntry entry;
        _queue->dequeue_wait(entry);
        _itemMap[entry.tid].emplace_back(entry);
        ++_totalConsumeItemCnt;
        ++dequeueItemCnt;
    }
    ORIGIN_DEBUG_INFO("Consumer thread finish. thread consume: {}, max: {}.",
                      dequeueItemCnt,
                      _totalProduceItemCnt);
}

void TestConcurrentBlockingQueueMt::ConsumerLoopWaitFor()
{
    ORIGIN_DEBUG_INFO("Consumer thread start. curr total consume: {}, max: {}.",
                      _totalConsumeItemCnt.load(),
                      _totalProduceItemCnt);
    uint32_t threadConsumeItemCnt = 0;
    while (_isRunning) {
        std::lock_guard lock(_mapMtx);
        TestEntry entry;
        if (_queue->dequeue_wait_for(entry)) {
            _itemMap[entry.tid].emplace_back(entry);
            ++_totalConsumeItemCnt;
            ++threadConsumeItemCnt;
        }
    }
    ORIGIN_DEBUG_INFO("Consumer thread finish. thread consume: {}, max: {}.",
                      threadConsumeItemCnt,
                      _totalProduceItemCnt);
}

void TestConcurrentBlockingQueueMt::StartThread(uint32_t producerCnt, uint32_t consumerCnt)
{
    for (uint32_t i = 0; i < producerCnt; i++) {
        _threads.emplace_back(&TestConcurrentBlockingQueueMt::ProducerLoop, this);
    }

    if (producerCnt > 1) {
        for (uint32_t i = 0; i < consumerCnt; i++) {
            _threads.emplace_back(&TestConcurrentBlockingQueueMt::ConsumerLoopWaitFor, this);
        }
    } else {
        _threads.emplace_back(&TestConcurrentBlockingQueueMt::ConsumerLoopWait, this);
    }
}

void TestConcurrentBlockingQueueMt::WaitFinish()
{
    while (_totalConsumeItemCnt < _totalProduceItemCnt) {
        sleep_ms(1);
    }
    _isRunning = false;
}

void TestConcurrentBlockingQueueMt::TestResult()
{
    EXPECT_EQ(_itemMap.size(), _producerCnt);
    for (auto& [tid, entries] : _itemMap) {
        EXPECT_EQ(entries.size(), MAX_NUM);
        for (uint32_t i = 0; i < entries.size(); i++) {
            EXPECT_EQ(entries[i].tid, tid);
            EXPECT_EQ(entries[i].num, i);
        }
    }
}

void TestConcurrentBlockingQueueMt::RunTest(uint32_t producerCnt, uint32_t consumerCnt,
                                            uint32_t capacity)
{
    _producerCnt = producerCnt;
    _totalProduceItemCnt = producerCnt * MAX_NUM;
    _totalConsumeItemCnt = 0;
    _queue = std::make_shared<ConcurrentBlockingQueue<TestEntry>>(capacity);
    StartThread(producerCnt, consumerCnt);
    WaitFinish();
    TestResult();
}

TEST_F(TestConcurrentBlockingQueueMt, SPSC)
{
    RunTest(1, 1);
}

TEST_F(TestConcurrentBlockingQueueMt, MPSC)
{
    RunTest(5, 1);
}

TEST_F(TestConcurrentBlockingQueueMt, SPMC)
{
    RunTest(1, 5);
}

TEST_F(TestConcurrentBlockingQueueMt, MPMC)
{
    RunTest(5, 5);
}

}  // namespace test::test_container::test_concurrent_queue
