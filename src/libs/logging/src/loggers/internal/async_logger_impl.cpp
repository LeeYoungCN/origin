#include "loggers/internal/async_logger_impl.h"

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include "internal/registry.h"
#include "internal/task_pool.h"
#include "loggers/internal/logger_impl_base.h"
#include "logging/log_msg.h"
#include "logging/sinks/sink.h"

namespace origin::logging {

AsyncLoggerImpl::AsyncLoggerImpl(std::string_view name, const std::shared_ptr<Sink>& sink)
    : LoggerImplBase(name, sink)
{
    if (REGISTRY.root_task_pool() == nullptr) {
        throw std::runtime_error(
            "Async logger requires a task pool to be registered in the registry.");
    }
    _taskPool = REGISTRY.root_task_pool();
}

AsyncLoggerImpl::AsyncLoggerImpl(std::string_view name,
                                 const std::vector<std::shared_ptr<Sink>>& sinks)
    : LoggerImplBase(name, sinks)
{
    if (REGISTRY.root_task_pool() == nullptr) {
        throw std::runtime_error(
            "Async logger requires a task pool to be registered in the registry.");
    }
    _taskPool = REGISTRY.root_task_pool();
}

AsyncLoggerImpl::AsyncLoggerImpl(std::string_view name,
                                 const std::initializer_list<std::shared_ptr<Sink>>& sinks)
    : LoggerImplBase(name, sinks)
{
    if (REGISTRY.root_task_pool() == nullptr) {
        throw std::runtime_error(
            "Async logger requires a task pool to be registered in the registry.");
    }
    _taskPool = REGISTRY.root_task_pool();
}

AsyncLoggerImpl::AsyncLoggerImpl(std::string_view name, const std::shared_ptr<Sink>& sink,
                                 const std::weak_ptr<TaskPool>& pool)
    : LoggerImplBase(name, sink), _taskPool(pool)
{
    if (pool.expired()) {
        throw std::invalid_argument("Task pool cannot be null.");
    }
}

AsyncLoggerImpl::AsyncLoggerImpl(std::string_view name,
                                 const std::vector<std::shared_ptr<Sink>>& sinks,
                                 const std::weak_ptr<TaskPool>& pool)
    : LoggerImplBase(name, sinks), _taskPool(pool)
{
    if (pool.expired()) {
        throw std::invalid_argument("Task pool cannot be null.");
    }
}

AsyncLoggerImpl::AsyncLoggerImpl(std::string_view name,
                                 const std::initializer_list<std::shared_ptr<Sink>>& sinks,
                                 const std::weak_ptr<TaskPool>& pool)
    : LoggerImplBase(name, sinks), _taskPool(pool)
{
    if (pool.expired()) {
        throw std::invalid_argument("Task pool cannot be null.");
    }
}

void AsyncLoggerImpl::log_it(const LogMsg& logMsg)
{
    _taskPool.lock()->log(shared_from_this(), logMsg);
}

void AsyncLoggerImpl::flush_it()
{
    _taskPool.lock()->flush(shared_from_this());
}
}  // namespace origin::logging
