#pragma once
#ifndef ORIGIN_LOGGING_INTERNAL_LOG_TASK_H
#define ORIGIN_LOGGING_INTERNAL_LOG_TASK_H

#include <memory>
#include <utility>

#include "internal/log_msg.h"

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
    explicit LogTask(TaskType type) : type(type) {}

    LogTask(TaskType type, const std::shared_ptr<LoggerImplBase>& logger)
        : type(type), logger(logger)
    {
    }

    LogTask(TaskType type, const std::shared_ptr<LoggerImplBase>& logger, LogMsg logMsg)
        : type(type), logMsg(std::move(logMsg)), logger(logger)
    {
    }
};

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_INTERNAL_LOG_TASK_H
