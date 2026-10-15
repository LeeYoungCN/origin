#include <algorithm>
#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "common/debug/debug_logger.h"
#include "common/types/date_time_types.h"
#include "common/types/thread_types.h"
#include "detail/common.hpp"
#include "detail/mock_sinks/tid_msg_map_sink.hpp"
#include "gtest/gtest.h"
#include "logging/log_level.hpp"
#include "logging/log_msg.hpp"
#include "logging/log_source.hpp"
#include "logging/loggers/async_logger.hpp"
#include "logging/loggers/logger.hpp"
#include "logging/loggers/sync_logger.hpp"
#include "logging/logging.hpp"
#include "logging/sinks/basic_file_sink.hpp"
#include "logging/sinks/sink.hpp"
#include "utils/date_time_utils.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"

using namespace logging_test;
using namespace origin::logging;
using namespace origin::thread;
using namespace origin::date_time;
using namespace origin::filesystem;

namespace {
constexpr auto TEST_LOG_LEVEL = LogLevel::ERR;

struct ProducerInfo {
    ThreadId tid{0};
    uint32_t idx;
    std::thread work_loop;

    ProducerInfo(uint32_t idx, std::thread thread) : idx(idx), work_loop(std::move(thread)) {}
};

LogSource TEST_LOG_SRC = {__FILE__, __LINE__, "function"};
}  // namespace

namespace logging_test {
class TestMPMC : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;

    void run_test(uint32_t producerCnt, uint32_t logCnt);
    void producer_loop(uint32_t idx, uint32_t producerCnt, uint32_t logCnt);
    void init_logger(const testing::TestInfo *test_info, uint32_t consumerCnt);
    static std::string format_log_msg(uint32_t producerIdx, uint32_t msgIdx);
    void wait_flush_complete(uint32_t producerCnt, uint32_t logCnt, uint32_t intervalMs,
                             uint32_t maxWaitTimeMs) const;

    std::string _dir = get_log_dir();
    std::shared_ptr<Logger> _logger;
    std::shared_ptr<TidMsgMapSink> _sink = std::make_shared<TidMsgMapSink>();
    std::vector<ProducerInfo> _producers;
    std::shared_ptr<TaskPool> _taskPool;
};

void TestMPMC::SetUp()
{
    _sink->set_level(LogLevel::TRACE);
    _sink->enable_debug_info(false);
}

void TestMPMC::TearDown()
{
    _logger.reset();
    _taskPool.reset();
    delete_dir(_dir);
}

std::string TestMPMC::format_log_msg(uint32_t producerIdx, uint32_t msgIdx)
{
    return std::format("Producer {}: {:04}.", producerIdx, msgIdx);
}

void TestMPMC::producer_loop(uint32_t idx, uint32_t producerCnt, uint32_t logCnt)
{
    _producers[idx - 1].tid = get_curr_thread_id();
    ORIGIN_DEBUG_DBG("Producer start. [{}/{}]", idx, producerCnt);
    for (uint32_t i = 0; i < logCnt; i++) {
        _logger->log(TEST_LOG_SRC, TEST_LOG_LEVEL, format_log_msg(idx, i));
    }
    ORIGIN_DEBUG_DBG("Producer stop. [{}/{}]", idx, producerCnt);
}

void TestMPMC::run_test(uint32_t producerCnt, uint32_t logCnt)
{
    for (uint32_t i = 1; i <= producerCnt; i++) {
        _producers.emplace_back(
            i, std::thread(&TestMPMC::producer_loop, this, i, producerCnt, logCnt));
    }

    for (auto &producer : _producers) {
        producer.work_loop.join();
    }

    wait_flush_complete(producerCnt, logCnt, 10, 500);

    EXPECT_EQ(_sink->buffer().size(), producerCnt);
    for (const ProducerInfo &producer : _producers) {
        std::vector<LogMsg> logMsgs = _sink->buffer().at(producer.tid);
        EXPECT_EQ(logMsgs.size(), logCnt);
        std::sort(logMsgs.begin(), logMsgs.end(), [](const LogMsg &a, const LogMsg &b) {
            return a.data < b.data;
        });
        for (uint32_t i = 0; i < logCnt; i++) {
            const LogMsg &logMsg = logMsgs.at(i);
            EXPECT_EQ(logMsg.threadId, producer.tid);
            EXPECT_EQ(logMsg.data, format_log_msg(producer.idx, i));
            EXPECT_EQ(logMsg.loggerName, _logger->name());
            EXPECT_EQ(logMsg.level, TEST_LOG_LEVEL);
            EXPECT_EQ(logMsg.source.file, TEST_LOG_SRC.file);
            EXPECT_EQ(logMsg.source.line, TEST_LOG_SRC.line);
            EXPECT_EQ(logMsg.source.func, TEST_LOG_SRC.func);
        }
    }
}

void TestMPMC::wait_flush_complete(uint32_t producerCnt, uint32_t logCnt, uint32_t intervalMs,
                                   uint32_t maxWaitTimeMs) const
{
    auto _isFlushComplete = [&]() -> bool {
        if (_sink->buffer().size() < producerCnt) {
            return false;
        }
        return std::ranges::all_of(
            _sink->buffer(), [logCnt](const auto &entry) { return entry.second.size() >= logCnt; });
    };

    uint32_t waitedTimeMs = 0;
    while (!_isFlushComplete()) {
        sleep_ms(intervalMs);
        waitedTimeMs += intervalMs;
        if (maxWaitTimeMs > 0 && waitedTimeMs >= maxWaitTimeMs) {
            ORIGIN_DEBUG_ERR("Wait flush complete timeout. waited: {}ms, max: {}ms.",
                             waitedTimeMs,
                             maxWaitTimeMs);
            break;
        }
    }
}

void TestMPMC::init_logger(const testing::TestInfo *test_info, uint32_t consumerCnt)
{
    const std::string name = get_logger_name(test_info);
    const std::string logFile = join_paths({_dir, name + ".log"});
    std::shared_ptr<Sink> const logSink = create_sink<BasicFileSink>(logFile);
    if (consumerCnt > 0) {
        _taskPool = create_task_pool(1024, consumerCnt);
        _logger = std::make_shared<AsyncLogger>(
            name, std::vector<std::shared_ptr<Sink>>{_sink, logSink}, _taskPool);

    } else {
        _logger =
            std::make_shared<SyncLogger>(name, std::vector<std::shared_ptr<Sink>>{_sink, logSink});
    }
    _logger->set_level(LogLevel::TRACE);
}

TEST_F(TestMPMC, Sync_SPSC)
{
    constexpr uint32_t logCnt = 2048;
    constexpr uint32_t producerCnt = 1;
    constexpr uint32_t consumerCnt = 0;
    init_logger(test_info_, consumerCnt);
    run_test(producerCnt, logCnt);
}

TEST_F(TestMPMC, Async_SPSC)
{
    constexpr uint32_t logCnt = 2048;
    constexpr uint32_t producerCnt = 1;
    constexpr uint32_t consumerCnt = 1;
    init_logger(test_info_, consumerCnt);
    run_test(producerCnt, logCnt);
}

TEST_F(TestMPMC, Sync_MPSC)
{
    constexpr uint32_t logCnt = 2048;
    constexpr uint32_t producerCnt = 5;
    constexpr uint32_t consumerCnt = 0;
    init_logger(test_info_, consumerCnt);
    run_test(producerCnt, logCnt);
}

TEST_F(TestMPMC, Async_MPSC)
{
    constexpr uint32_t logCnt = 2048;
    constexpr uint32_t producerCnt = 5;
    constexpr uint32_t consumerCnt = 1;
    init_logger(test_info_, consumerCnt);
    run_test(producerCnt, logCnt);
}

TEST_F(TestMPMC, Async_SPMC)
{
    constexpr uint32_t logCnt = 2048;
    constexpr uint32_t producerCnt = 1;
    constexpr uint32_t consumerCnt = 5;
    init_logger(test_info_, consumerCnt);
    run_test(producerCnt, logCnt);
}

TEST_F(TestMPMC, Async_MPMC)
{
    constexpr uint32_t logCnt = 2048;
    constexpr uint32_t producerCnt = 5;
    constexpr uint32_t consumerCnt = 5;
    init_logger(test_info_, consumerCnt);
    run_test(producerCnt, logCnt);
}

}  // namespace logging_test
