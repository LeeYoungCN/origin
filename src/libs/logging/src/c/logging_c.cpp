#include "logging/c/logging_c.h"

#include <cstdarg>

#include "c/common_c.h"
#include "internal/common.h"
#include "internal/registry.h"

using namespace origin::logging;
using namespace origin::logging::c;

#define ROOT_LOGGER (INST(Registry).root_logger())

extern "C" {
LoggerSt *origin_root_logger()
{
    return new struct LoggerSt(ROOT_LOGGER);
}

void origin_set_root_logger(const LoggerSt *logger)
{
    RETURN_IF_PTR_NULL(logger);
    RETURN_IF_PTR_NULL(logger->ptr);
    REGISTRY.set_root_logger(logger->ptr);
}

void origin_set_level(OriginLogLevel level)
{
    ROOT_LOGGER->set_level(c_to_cpp_log_level(level));
}

bool origin_should_log(OriginLogLevel level)
{
    return ROOT_LOGGER->should_log(c_to_cpp_log_level(level));
}

OriginLogLevel origin_level()
{
    return cpp_to_c_log_level(ROOT_LOGGER->level());
}

void origin_flush_on(OriginLogLevel level)
{
    ROOT_LOGGER->flush_on(c_to_cpp_log_level(level));
}

bool origin_should_flush(OriginLogLevel level)
{
    return ROOT_LOGGER->should_flush(c_to_cpp_log_level(level));
}

OriginLogLevel origin_flush_level()
{
    return cpp_to_c_log_level(ROOT_LOGGER->flush_level());
}

void origin_set_pattern(const char *pattern)
{
    RETURN_IF_PTR_NULL(pattern);
    return ROOT_LOGGER->set_pattern(pattern);
}

void origin_set_formatter(const FormatterSt *formatter)
{
    RETURN_IF_PTR_NULL(formatter);
    RETURN_IF_PTR_NULL(formatter->ptr);
    return ROOT_LOGGER->set_formatter(formatter->ptr);
}

void origin_flush()
{
    ROOT_LOGGER->flush();
}

void origin_force_log(const char *file, int line, const char *func, OriginLogLevel level,
                      const char *format, ...)
{
    va_list args;
    va_start(args, format);
    origin_force_log_it(ROOT_LOGGER, file, line, func, c_to_cpp_log_level(level), format, args);
    va_end(args);
}

void origin_log(const char *file, int line, const char *func, OriginLogLevel level,
                const char *format, ...)
{
    va_list args;
    va_start(args, format);
    origin_log_it(ROOT_LOGGER, file, line, func, c_to_cpp_log_level(level), format, args);
    va_end(args);
}

bool origin_register_logger(const LoggerSt *logger)
{
    RETURN_VALUE_IF_PTR_NULL(logger, false);

    return REGISTRY.register_logger(logger->ptr);
}

void origin_register_or_replace_logger(const LoggerSt *logger)
{
    RETURN_IF_PTR_NULL(logger);

    REGISTRY.register_or_replace_logger(logger->ptr);
}

void origin_remove_logger(const char *name)
{
    REGISTRY.remove_logger(name);
}

void origin_remove_all()
{
    REGISTRY.remove_all();
}

LoggerSt *origin_get_logger(const char *name)
{
    auto logger = REGISTRY.get_logger(name);
    if (logger == nullptr) {
        return nullptr;
    } else {
        return new struct LoggerSt(logger);
    }
}

void origin_init_root_task_pool(uint32_t capacity, uint32_t threadCnt)
{
    REGISTRY.init_root_task_pool(capacity, threadCnt);
}

TaskPoolSt *origin_root_task_pool()
{
    auto taskPool = REGISTRY.root_task_pool();
    if (taskPool == nullptr) {
        return nullptr;
    } else {
        return new struct TaskPoolSt(taskPool);
    }
}

void origin_initialize_logger(LoggerSt const *logger, bool autoRegister)
{
    RETURN_IF_PTR_NULL(logger);
    REGISTRY.initialize_logger(logger->ptr, autoRegister);
}

void origin_set_level_all(OriginLogLevel level)
{
    REGISTRY.set_level_all(c_to_cpp_log_level(level));
}

void origin_flush_on_all(OriginLogLevel level)
{
    REGISTRY.flush_on_all(c_to_cpp_log_level(level));
}

void origin_set_pattern_all(const char *pattern)
{
    RETURN_IF_PTR_NULL(pattern);
    REGISTRY.set_pattern_all(pattern);
}

void origin_set_formatter_all(FormatterSt const *formatter)
{
    RETURN_IF_PTR_NULL(formatter);
    RETURN_IF_PTR_NULL(formatter->ptr);
    REGISTRY.set_formatter_all(formatter->ptr->clone());
}

void origin_flush_all()
{
    REGISTRY.flush_all();
}

void origin_shutdown()
{
    REGISTRY.shutdown();
}
}
