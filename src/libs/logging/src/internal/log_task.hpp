
#ifndef ORIGIN_LOGGING_INTERNAL_LOG_TASK_HPP
#define ORIGIN_LOGGING_INTERNAL_LOG_TASK_HPP

#include <memory>
#include <utility>

#include "logging/log_msg.hpp"

namespace origin::logging {
class LoggerImplBase;
enum class TaskType {
    LOG,
    FLUSH,
    SHUTDOWN,
};

struct LogTask {
    TaskType type = TaskType::SHUTDOWN;
    LogMsg logMsg;
    std::shared_ptr<LoggerImplBase> logger;

    LogTask() = default;
    explicit LogTask(const TaskType type) : type(type) {}

    LogTask(const TaskType type, const std::shared_ptr<LoggerImplBase>& logger)
        : type(type), logger(logger)
    {
    }

    LogTask(const TaskType type, const std::shared_ptr<LoggerImplBase>& logger, LogMsg logMsg)
        : type(type), logMsg(std::move(logMsg)), logger(logger)
    {
    }
};

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_INTERNAL_LOG_TASK_HPP
