#include "logging/logging.hpp"

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "c/internal/common_c.hpp"
#include "common/debug/debug_logger.h"
#include "internal/registry.hpp"
#include "internal/task_pool.hpp"
#include "logging/formatters/formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/loggers/async_logger.hpp"
#include "logging/loggers/logger.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {
std::shared_ptr<Logger> create_async_logger(const std::string_view name,
                                            const std::shared_ptr<Sink>& sink,
                                            const std::weak_ptr<TaskPool>& pool)
{
    return std::make_shared<AsyncLogger>(name, sink, pool);
}

std::shared_ptr<Logger> create_async_logger(
    const std::string_view name, const std::initializer_list<std::shared_ptr<Sink>> sinks,
    const std::weak_ptr<TaskPool>& pool)
{
    return std::make_shared<AsyncLogger>(name, sinks, pool);
}

std::shared_ptr<TaskPool> create_task_pool(const uint32_t capacity, const uint32_t threadCnt)
{
    return std::make_shared<TaskPool>(capacity, threadCnt);
}

#pragma region root logger
std::shared_ptr<Logger> root_logger()
{
    return REGISTRY.root_logger();
}

Logger* root_logger_raw()
{
    return REGISTRY.root_logger_raw();
}

void set_root_logger(std::shared_ptr<Logger> logger)
{
    REGISTRY.set_root_logger(std::move(logger));
}

void set_level(const LogLevel level)
{
    if (!root_logger_raw()) {
        ORIGIN_DEBUG_ERR("Root logger set log level failed. {}", ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
        return;
    }
    root_logger_raw()->set_level(level);
}

LogLevel level()
{
    if (!root_logger_raw()) {
        ORIGIN_DEBUG_ERR("Root logger get log level failed. {}", ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
        return LogLevel::OFF;
    }
    return root_logger_raw()->level();
}

bool should_log(const LogLevel level)
{
    if (!root_logger_raw()) {
        ORIGIN_DEBUG_ERR("Root logger should log failed. {}", ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
        return false;
    }
    return root_logger_raw()->should_log(level);
}

void flush_on(const LogLevel level)
{
    if (!root_logger_raw()) {
        ORIGIN_DEBUG_ERR("Root logger flush on failed. {}", ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
        return;
    }
    root_logger_raw()->flush_on(level);
}

LogLevel flush_level()
{
    if (!root_logger_raw()) {
        ORIGIN_DEBUG_ERR("Root logger get flush level failed. {}", ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
        return LogLevel::OFF;
    }
    return root_logger_raw()->flush_level();
}

bool should_flush(const LogLevel level)
{
    if (!root_logger_raw()) {
        ORIGIN_DEBUG_ERR("Root logger should flush failed. {}", ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
        return false;
    }
    return root_logger_raw()->should_flush(level);
}

void set_pattern(const std::string_view pattern)
{
    if (!root_logger_raw()) {
        ORIGIN_DEBUG_ERR("Root logger set pattern failed. {}", ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
        return;
    }
    root_logger_raw()->set_pattern(pattern);
}

void set_formatter(const std::unique_ptr<Formatter>& formatter)
{
    if (!root_logger_raw()) {
        ORIGIN_DEBUG_ERR("Root logger set formatter failed. {}", ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
        return;
    }
    root_logger_raw()->set_formatter(formatter);
}

void flush()
{
    if (!root_logger_raw()) {
        ORIGIN_DEBUG_ERR("Root logger flush failed. {}", ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
        return;
    }
    root_logger_raw()->flush();
}

#pragma endregion

#pragma region logging manager
void initialize_logger(const std::shared_ptr<Logger>& logger)
{
    REGISTRY.initialize_logger(logger);
}

void set_level_all(const LogLevel level)
{
    REGISTRY.set_level_all(level);
}

void flush_on_all(const LogLevel level)
{
    REGISTRY.flush_on_all(level);
}

void set_pattern_all(const std::string_view pattern)
{
    REGISTRY.set_pattern_all(pattern);
}

void set_formatter_all(std::unique_ptr<Formatter> formatter)
{
    REGISTRY.set_formatter_all(std::move(formatter));
}

void flush_all()
{
    REGISTRY.flush_all();
}

void shutdown()
{
    REGISTRY.shutdown();
}
#pragma endregion

#pragma region registry
bool register_logger(std::shared_ptr<Logger> logger)
{
    return REGISTRY.register_logger(std::move(logger));
}

void register_or_replace_logger(std::shared_ptr<Logger> logger)
{
    REGISTRY.register_or_replace_logger(std::move(logger));
}

void remove_logger(const std::string_view name)
{
    REGISTRY.remove_logger(name);
}

void remove_all()
{
    REGISTRY.remove_all();
}

std::shared_ptr<Logger> get_logger(const std::string_view name)
{
    return REGISTRY.get_logger(name);
}

void init_root_task_pool(uint32_t capacity, uint32_t threadCnt)
{
    REGISTRY.init_root_task_pool(capacity, threadCnt);
}

void set_root_task_pool(std::shared_ptr<TaskPool> taskPool)
{
    REGISTRY.set_root_task_pool(std::move(taskPool));
}

std::shared_ptr<TaskPool> root_task_pool()
{
    return REGISTRY.root_task_pool();
}
#pragma endregion

}  // namespace origin::logging
