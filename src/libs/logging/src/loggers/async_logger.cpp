#include "logging/loggers/async_logger.hpp"

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include "internal/task_pool.hpp"
#include "loggers/internal/async_logger_impl.hpp"
#include "logging/loggers/logger_base.hpp"

namespace origin::logging {

AsyncLogger::AsyncLogger(std::string_view name, const std::shared_ptr<Sink>& sink)
    : LoggerBase(std::make_shared<AsyncLoggerImpl>(name, sink))
{
}

AsyncLogger::AsyncLogger(std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks)
    : LoggerBase(std::make_shared<AsyncLoggerImpl>(name, sinks.begin(), sinks.end()))
{
}

AsyncLogger::AsyncLogger(std::string_view name,
                         const std::initializer_list<std::shared_ptr<Sink>>& sinks)
    : LoggerBase(std::make_shared<AsyncLoggerImpl>(name, sinks.begin(), sinks.end()))
{
}

AsyncLogger::AsyncLogger(std::string_view name, const std::shared_ptr<Sink>& sink,
                         const std::weak_ptr<TaskPool>& pool)
    : LoggerBase(std::make_shared<AsyncLoggerImpl>(name, sink, pool))
{
}

AsyncLogger::AsyncLogger(std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks,
                         const std::weak_ptr<TaskPool>& pool)
    : LoggerBase(std::make_shared<AsyncLoggerImpl>(name, sinks.begin(), sinks.end(), pool))
{
}

AsyncLogger::AsyncLogger(std::string_view name,
                         const std::initializer_list<std::shared_ptr<Sink>>& sinks,
                         const std::weak_ptr<TaskPool>& pool)
    : LoggerBase(std::make_shared<AsyncLoggerImpl>(name, sinks.begin(), sinks.end(), pool))
{
}

}  // namespace origin::logging
