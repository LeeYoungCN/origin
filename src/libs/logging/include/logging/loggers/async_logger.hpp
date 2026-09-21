#ifndef ORIGIN_LOGGING_LOGGERS_ASYNC_LOGGER_HPP
#define ORIGIN_LOGGING_LOGGERS_ASYNC_LOGGER_HPP

#include <memory>

#include "logging/detail/logging_api.h"
#include "logging/loggers/logger_base.hpp"

namespace origin::logging {
class TaskPool;

class LOGGING_API AsyncLogger : public LoggerBase {
public:
    AsyncLogger() = delete;
    ~AsyncLogger() override = default;

    AsyncLogger(std::string_view name, const std::shared_ptr<Sink>& sink);

    AsyncLogger(std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks);

    AsyncLogger(std::string_view name, const std::initializer_list<std::shared_ptr<Sink>>& sinks);

    AsyncLogger(std::string_view name, const std::shared_ptr<Sink>& sink,
                const std::weak_ptr<TaskPool>& pool);

    AsyncLogger(std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks,
                const std::weak_ptr<TaskPool>& pool);

    AsyncLogger(std::string_view name, const std::initializer_list<std::shared_ptr<Sink>>& sinks,
                const std::weak_ptr<TaskPool>& pool);
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGERS_ASYNC_LOGGER_HPP
