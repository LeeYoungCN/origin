#ifndef ORIGIN_LOGGING_LOGGERS_INTERNAL_ASYNC_LOGGER_IMPL_HPP
#define ORIGIN_LOGGING_LOGGERS_INTERNAL_ASYNC_LOGGER_IMPL_HPP

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

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
    AsyncLoggerImpl(std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks);
    AsyncLoggerImpl(std::string_view name,
                    const std::initializer_list<std::shared_ptr<Sink>>& sinks);

    AsyncLoggerImpl(std::string_view name, const std::shared_ptr<Sink>& sink,
                    const std::weak_ptr<TaskPool>& pool);
    AsyncLoggerImpl(std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks,
                    const std::weak_ptr<TaskPool>& pool);
    AsyncLoggerImpl(std::string_view name,
                    const std::initializer_list<std::shared_ptr<Sink>>& sinks,
                    const std::weak_ptr<TaskPool>& pool);

protected:
    void log_it(const LogMsg& logMsg) override;
    void flush_it() override;

private:
    std::weak_ptr<TaskPool> _taskPool;
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGERS_INTERNAL_ASYNC_LOGGER_IMPL_HPP
