#include "internal/task_pool.h"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

#include "container/concurrent_blocking_queue.hpp"
#include "common/debug/debug_logger.h"
#include "internal/log_msg.h"
#include "internal/log_task.h"
#include "loggers/internal/logger_impl_base.h"

namespace origin::logging {
using namespace origin::container;

TaskPool::TaskPool() : TaskPool(TaskPool::DEFAULT_CAPACITY, TaskPool::DEFAULT_THREAD_CNT) {}

TaskPool::TaskPool(uint32_t capacity) : TaskPool(capacity, TaskPool::DEFAULT_THREAD_CNT) {}

TaskPool::TaskPool(uint32_t capacity, uint32_t threadCnt)
    : _buffer(ConcurrentBlockingQueue<LogTask>(capacity)),
      _threadCnt(threadCnt),
      _paramStr(std::format("Capacity: {}, Thread count: {}.", capacity, threadCnt))
{
    ORIGIN_DEBUG_DBG("Create task pool. {}", _paramStr);
    if (capacity == 0) {
        throw std::invalid_argument("Capacity must be greater than zero.");
    }

    if (_threadCnt == 0) {
        throw std::invalid_argument("Thread count must be greater than zero.");
    }
    _threadPool.reserve(_threadCnt);
    start();
}

TaskPool::~TaskPool()
{
    shutdown();
    ORIGIN_DEBUG_DBG("Release task pool. {}", _paramStr);
}

void TaskPool::log(const std::shared_ptr<LoggerImplBase>& logger, const LogMsg& logMsg)
{
    _buffer.enqueue_wait(LogTask(TaskType::LOG, logger, logMsg));
}

void TaskPool::flush(const std::shared_ptr<LoggerImplBase>& logger)
{
    _buffer.enqueue_wait(LogTask(TaskType::FLUSH, logger, LogMsg()));
}

void TaskPool::start()
{
    for (uint32_t i = 0; i < _threadCnt; i++) {
        _threadPool.emplace_back(&TaskPool::worker_loop, this, i + 1);
    }

    ORIGIN_DEBUG_TRACE("Task pool start.");
}

void TaskPool::shutdown()
{
    for (uint32_t i = 0; i < _threadPool.size(); i++) {
        _buffer.enqueue_wait(LogTask(TaskType::SHUTDOWN));
    }

    for (auto& t : _threadPool) {
        if (t.joinable()) {
            t.join();
        }
    }

    _threadPool.clear();

    ORIGIN_DEBUG_TRACE("Task pool shutdown.");
}

void TaskPool::worker_loop(uint32_t idx)
{
    ORIGIN_DEBUG_DBG("Log thread pool worker loop start. [{}/{}]", idx, _threadCnt);
    bool isRunning = true;
    while (isRunning) {
        LogTask task;
        _buffer.dequeue_wait(task);

        switch (task.type) {
            case TaskType::LOG:
                task.logger->backend_log(task.logMsg);
                break;
            case TaskType::FLUSH:
                task.logger->backend_flush();
                break;
            case TaskType::SHUTDOWN:
            default:
                isRunning = false;
                break;
        }
    }
    ORIGIN_DEBUG_DBG("Log task pool worker loop shutdown. [{}/{}]", idx, _threadCnt);
}

std::string_view TaskPool::param_str()
{
    return _paramStr;
}
}  // namespace origin::logging
