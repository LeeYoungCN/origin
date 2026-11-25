#include "origin/logging/logging.hpp"

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "origin/common/debug/debug_logger.h"

#include "origin/logging/formatters/formatter.hpp"
#include "origin/logging/log_level.hpp"
#include "origin/logging/log_source.hpp"
#include "origin/logging/loggers/async_logger.hpp"
#include "origin/logging/loggers/logger.hpp"
#include "origin/logging/sinks/sink.hpp"

#include "c/internal/common_c.hpp"
#include "internal/registry.hpp"
#include "internal/task_pool.hpp"

namespace origin::logging {
namespace {
std::shared_ptr<Logger> get_root_logger_or_throw(const char* action)
{
    auto logger = root_logger();
    if (!logger) {
        ORIGIN_DEBUG_ERR("Root logger {} failed. {}", action, ROOT_LOGGER_NULL_LOG);
        throw std::runtime_error(ROOT_LOGGER_NULL_LOG.data());
    }
    return logger;
}
}  // namespace

std::shared_ptr<Logger> create_async_logger(const std::string_view name,
                                            const std::shared_ptr<Sink>& sink,
                                            const std::weak_ptr<TaskPool>& pool)
{
    return std::make_shared<AsyncLogger>(name, sink, pool);
}

std::shared_ptr<Logger> create_async_logger(
    const std::string_view name,
    const std::initializer_list<std::shared_ptr<Sink>> sinks,
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

void set_root_logger(std::shared_ptr<Logger> logger)
{
    REGISTRY.set_root_logger(std::move(logger));
}

void set_level(const LogLevel level)
{
    get_root_logger_or_throw("set log level")->set_level(level);
}

LogLevel level()
{
    return get_root_logger_or_throw("get log level")->level();
}

bool should_log(const LogLevel level)
{
    return get_root_logger_or_throw("should log")->should_log(level);
}

void flush_on(const LogLevel level)
{
    get_root_logger_or_throw("flush on")->flush_on(level);
}

LogLevel flush_level()
{
    return get_root_logger_or_throw("get flush level")->flush_level();
}

bool should_flush(const LogLevel level)
{
    return get_root_logger_or_throw("should flush")->should_flush(level);
}

void set_pattern(const std::string_view pattern)
{
    get_root_logger_or_throw("set pattern")->set_pattern(pattern);
}

void set_formatter(const std::unique_ptr<Formatter>& formatter)
{
    get_root_logger_or_throw("set formatter")->set_formatter(formatter);
}

void flush()
{
    get_root_logger_or_throw("flush")->flush();
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

uint32_t log_all(const LogSource& source, LogLevel level, std::string_view message)
{
    uint32_t count = 0;
    apply_all([&](const std::shared_ptr<Logger>& logger) {
        if (logger->should_log(level)) {
            logger->force_log(source, level, message);
            ++count;
        }
    });
    return count;
}

void flush_all()
{
    apply_all([](const std::shared_ptr<Logger>& logger) { logger->flush(); });
}

void apply_all(const std::function<void(const std::shared_ptr<Logger>&)>& fun)
{
    if (fun == nullptr) {
        ORIGIN_DEBUG_ERR("Apply all failed. fun nullptr.");
        return;
    }

    REGISTRY.apply_all(fun);
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

bool is_logger_exists(const std::string_view name)
{
    return REGISTRY.exists(name);
}

std::vector<std::shared_ptr<Logger>> snapshot_loggers()
{
    return REGISTRY.snapshot_loggers();
}

bool set_root_task_pool(std::shared_ptr<TaskPool> taskPool)
{
    return REGISTRY.set_root_task_pool(std::move(taskPool));
}

std::shared_ptr<TaskPool> root_task_pool()
{
    return REGISTRY.root_task_pool();
}
#pragma endregion

}  // namespace origin::logging
