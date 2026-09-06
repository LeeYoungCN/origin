#ifndef ORIGIN_LOGGING_LOGGERS_INTERNAL_ASYNC_LOGGER_IMPL_H
#define ORIGIN_LOGGING_LOGGERS_INTERNAL_ASYNC_LOGGER_IMPL_H

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include "internal/log_msg.h"
#include "internal/task_pool.h"
#include "loggers/internal/logger_impl_base.h"
#include "logging/sinks/sink.h"

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

#endif  // ORIGIN_LOGGING_LOGGERS_INTERNAL_ASYNC_LOGGER_IMPL_H
