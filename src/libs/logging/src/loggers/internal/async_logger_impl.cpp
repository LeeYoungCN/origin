#include "loggers/internal/async_logger_impl.hpp"

#include <memory>
#include <string_view>

#include "common/debug/debug_logger.h"
#include "internal/registry.hpp"
#include "internal/task_pool.hpp"
#include "loggers/internal/logger_impl_base.hpp"
#include "logging/log_msg.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {

AsyncLoggerImpl::AsyncLoggerImpl(const std::string_view name, const std::shared_ptr<Sink>& sink)
    : LoggerImplBase(name, sink), _taskPool(REGISTRY.root_task_pool())
{
    throw_if_task_pool_invalid(true);
}

AsyncLoggerImpl::AsyncLoggerImpl(const std::string_view name, const std::shared_ptr<Sink>& sink,
                                 const std::weak_ptr<TaskPool>& pool)
    : LoggerImplBase(name, sink), _taskPool(pool)
{
    throw_if_task_pool_invalid(false);
}

void AsyncLoggerImpl::throw_if_task_pool_invalid(const bool useRoot)
{
    if (_taskPool.expired()) {
        if (useRoot) {
            constexpr const char* ERROR_INFO =
                "Async logger requires a task pool to be registered in the registry.";
            ORIGIN_DEBUG_ERR("Create logger failed. Name: {}. {}", _name, ERROR_INFO);
            throw std::runtime_error(ERROR_INFO);
        } else {
            constexpr const char* ERROR_INFO = "Task pool cannot be nullptr.";
            ORIGIN_DEBUG_ERR("Create logger failed. Name: {}. {}", _name, ERROR_INFO);
            throw std::invalid_argument(ERROR_INFO);
        }
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
