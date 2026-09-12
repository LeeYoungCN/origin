#pragma once
#ifndef ORIGIN_LOGGING_C_INTERNAL_COMMON_C_HPP
#define ORIGIN_LOGGING_C_INTERNAL_COMMON_C_HPP

#include <memory>
#include <vector>

#include "internal/task_pool.hpp"
#include "logging/c/logging_c.h"
#include "logging/formatters/formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/loggers/logger.hpp"
#include "logging/sinks/sink.hpp"

struct LoggerSt {
    std::shared_ptr<origin::logging::Logger> ptr;

    explicit LoggerSt(std::shared_ptr<origin::logging::Logger> ptr) : ptr(std::move(ptr)) {}
    explicit LoggerSt(origin::logging::Logger *ptr) : ptr(ptr) {}
};

struct SinkSt {
    std::shared_ptr<origin::logging::Sink> ptr;

    explicit SinkSt(std::shared_ptr<origin::logging::Sink> sink) : ptr(std::move(sink)) {}
    explicit SinkSt(origin::logging::Sink *sink) : ptr(sink) {}
};

struct FormatterSt {
    std::unique_ptr<origin::logging::Formatter> ptr;

    explicit FormatterSt(std::unique_ptr<origin::logging::Formatter> formatter)
        : ptr(std::move(formatter))
    {
    }
    explicit FormatterSt(origin::logging::Formatter *formatter) : ptr(formatter) {}
};

struct TaskPoolSt {
    std::shared_ptr<origin::logging::TaskPool> ptr;

    explicit TaskPoolSt(std::shared_ptr<origin::logging::TaskPool> taskPool)
        : ptr(std::move(taskPool))
    {
    }
    explicit TaskPoolSt(origin::logging::TaskPool *taskPool) : ptr(taskPool) {}
};

namespace origin::logging::c {
LogLevel c_to_cpp_log_level(OriginLogLevel level);
OriginLogLevel cpp_to_c_log_level(LogLevel level);
std::vector<std::shared_ptr<origin::logging::Sink>> sink_ptr_vector(const SinkSt *const sinks[],
                                                                    uint32_t sinkCnt);

void origin_force_log_it(const std::shared_ptr<origin::logging::Logger> &logger, const char *file,
                         int line, const char *func, LogLevel level, const char *format,
                         va_list args);

void origin_log_it(const std::shared_ptr<origin::logging::Logger> &logger, const char *file,
                   int line, const char *func, LogLevel level, const char *format, va_list args);
}  // namespace origin::logging::c
#endif  // ORIGIN_LOGGING_C_INTERNAL_COMMON_C_HPP
