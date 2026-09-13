#include "logging/logging.hpp"

#include <utility>

#include "internal/registry.hpp"
#include "internal/task_pool.hpp"

namespace origin::logging {
std::shared_ptr<Logger> create_async_logger(std::string_view name,
                                            const std::shared_ptr<Sink>& sink,
                                            const std::weak_ptr<TaskPool>& pool)
{
    return std::make_shared<AsyncLogger>(name, sink, pool);
}

std::shared_ptr<Logger> create_async_logger(std::string_view name,
                                            std::initializer_list<std::shared_ptr<Sink>> sinks,
                                            const std::weak_ptr<TaskPool>& pool)
{
    return std::make_shared<AsyncLogger>(name, sinks, pool);
}

std::shared_ptr<TaskPool> create_task_pool(uint32_t capacity, uint32_t threadCnt)
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

bool should_log(LogLevel level)
{
    return root_logger_raw()->should_log(level);
}

void set_level(LogLevel level)
{
    root_logger_raw()->set_level(level);
}

void flush_on(LogLevel level)
{
    root_logger_raw()->flush_on(level);
}

void set_pattern(std::string_view pattern)
{
    root_logger_raw()->set_pattern(pattern);
}

void set_formatter(const std::unique_ptr<Formatter>& formatter)
{
    root_logger_raw()->set_formatter(formatter);
}

void flush()
{
    root_logger_raw()->flush();
}

#pragma endregion

#pragma region logging manager
void initialize_logger(const std::shared_ptr<Logger>& logger)
{
    REGISTRY.initialize_logger(logger);
}

void set_level_all(LogLevel level)
{
    REGISTRY.set_level_all(level);
}

void flush_on_all(LogLevel level)
{
    REGISTRY.flush_on_all(level);
}

void set_pattern_all(std::string_view pattern)
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

void remove_logger(std::string_view name)
{
    REGISTRY.remove_logger(name);
}

void remove_all()
{
    REGISTRY.remove_all();
}

std::shared_ptr<Logger> get_logger(std::string_view name)
{
    return REGISTRY.get_logger(name);
}

void init_root_task_pool(uint32_t capacity, uint32_t threadCnt)
{
    REGISTRY.init_root_task_pool(capacity, threadCnt);
}

std::shared_ptr<TaskPool> root_task_pool()
{
    return REGISTRY.root_task_pool();
}
#pragma endregion

}  // namespace origin::logging
