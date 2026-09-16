#ifndef ORIGIN_LOGGING_LOGGERS_INTERNAL_ASYNC_LOGGER_IMPL_HPP
#define ORIGIN_LOGGING_LOGGERS_INTERNAL_ASYNC_LOGGER_IMPL_HPP

#include <memory>
#include <string_view>

#include "internal/registry.hpp"
#include "internal/task_pool.hpp"
#include "loggers/internal/logger_impl_base.hpp"
#include "logging/log_msg.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {
class AsyncLoggerImpl : public LoggerImplBase,
                        public std::enable_shared_from_this<AsyncLoggerImpl> {
public:
    AsyncLoggerImpl() = delete;
    ~AsyncLoggerImpl() override = default;

    AsyncLoggerImpl(std::string_view name, const std::shared_ptr<Sink>& sink);

    template <typename It>
    AsyncLoggerImpl(std::string_view name, It begin, It end)
        : LoggerImplBase(name, begin, end), _taskPool(REGISTRY.root_task_pool())
    {
        throw_if_task_pool_invalid(true);
    }

    AsyncLoggerImpl(std::string_view name, const std::shared_ptr<Sink>& sink,
                    const std::weak_ptr<TaskPool>& pool);

    template <typename It>
    AsyncLoggerImpl(std::string_view name, It begin, It end, const std::weak_ptr<TaskPool>& pool)
        : LoggerImplBase(name, begin, end), _taskPool(pool)
    {
        throw_if_task_pool_invalid(false);
    }

protected:
    void log_it(const LogMsg& logMsg) override;
    void flush_it() override;
    void throw_if_task_pool_invalid(bool useRoot);

private:
    std::weak_ptr<TaskPool> _taskPool;
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGERS_INTERNAL_ASYNC_LOGGER_IMPL_HPP
