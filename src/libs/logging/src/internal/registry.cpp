#include "internal/registry.h"

#include <memory>
#include <mutex>
#include <ranges>
#include <utility>

#include "common/debug/debug_logger.h"
#include "internal/common.h"
#include "logging/formatters/pattern_formatter.h"
#include "logging/loggers/sync_logger.h"
#include "logging/sinks/stdout_sink.h"

namespace origin::logging {

Registry::Registry() : _globalFormatter(new PatternFormatter())
{
    _rootLogger = std::make_shared<SyncLogger>(ROOT_LOGGER_NAME, std::make_shared<StdoutSink>());
    _loggers[ROOT_LOGGER_NAME] = _rootLogger;
}

#pragma region root logger
std::shared_ptr<Logger> Registry::root_logger()
{
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    return _rootLogger;
}

Logger* Registry::root_logger_raw()
{
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    return _rootLogger.get();
}

void Registry::set_root_logger(std::shared_ptr<Logger> newLogger)
{
    RETURN_IF_PTR_NULL(newLogger);
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    register_or_replace_logger_it(newLogger);
    _rootLogger = std::move(newLogger);
}
#pragma endregion

#pragma region logging manager
void Registry::initialize_logger(const std::shared_ptr<Logger>& logger, bool autoRegister)
{
    RETURN_IF_PTR_NULL(logger);
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    logger->set_formatter(_globalFormatter->clone());
    logger->set_level(_globalLevel);
    logger->flush_on(_globalFlushLevel);

    if (autoRegister) {
        register_or_replace_logger_it(logger);
    }
}

void Registry::set_level_all(LogLevel level)
{
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    _globalLevel = level;

    for (const auto& logger : _loggers | std::views::values) {
        logger->set_level(_globalLevel);
    }
}

void Registry::flush_on_all(LogLevel level)
{
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    _globalFlushLevel = level;

    for (const auto& logger : _loggers | std::views::values) {
        logger->flush_on(_globalFlushLevel);
    }
}

void Registry::set_pattern_all(std::string_view pattern)
{
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    set_formatter_all(std::make_unique<PatternFormatter>(pattern));
}

void Registry::set_formatter_all(std::unique_ptr<Formatter> formatter)
{
    RETURN_IF_PTR_NULL(formatter);
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    _globalFormatter = std::move(formatter);
    for (const auto& logger : _loggers | std::views::values) {
        logger->set_formatter(_globalFormatter->clone());
    }
}

void Registry::flush_all()
{
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    for (const auto& logger : _loggers | std::views::values) {
        logger->flush();
    }
}

void Registry::shutdown()
{
    remove_all();
    {
        std::lock_guard<std::recursive_mutex> const lock(_taskPoolMtx);
        if (_rootTaskPool != nullptr) {
            _rootTaskPool.reset();
        }
    }
    ORIGIN_DEBUG_DBG("Logging module shutdown.");
}
#pragma endregion

#pragma region registry
bool Registry::register_logger(std::shared_ptr<Logger> logger)
{
    RETURN_VALUE_IF_PTR_NULL(logger, false);
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    return register_logger_it(std::move(logger));
}

void Registry::register_or_replace_logger(std::shared_ptr<Logger> logger)
{
    RETURN_IF_PTR_NULL(logger);
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    register_or_replace_logger_it(std::move(logger));
}

void Registry::remove_logger(std::string_view name)
{
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    const bool isDefaultLogger = _rootLogger != nullptr && _rootLogger->name() == name;
    _loggers.erase(name);
    if (isDefaultLogger) {
        _rootLogger.reset();
    }
}

void Registry::remove_all()
{
    {
        std::lock_guard<std::mutex> const lock(_loggerMapMtx);
        if (_rootLogger != nullptr) {
            _rootLogger.reset();
        }
        _loggers.clear();
    }
    ORIGIN_DEBUG_DBG("Remove all loggers.");
}

std::shared_ptr<Logger> Registry::get_logger(std::string_view name)
{
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    auto it = _loggers.find(name);
    return it == _loggers.end() ? nullptr : it->second;
}

bool Registry::exist(std::string_view name)
{
    std::lock_guard<std::mutex> const lock(_loggerMapMtx);
    return exist_it(name);
}

void Registry::init_root_task_pool(uint32_t capacity, uint32_t threadCnt)
{
    std::lock_guard<std::recursive_mutex> const lock(_taskPoolMtx);
    if (_rootTaskPool != nullptr) {
        ORIGIN_DEBUG_ERR("Task pool already initialized.");
        return;
    }
    _rootTaskPool = std::make_shared<TaskPool>(capacity, threadCnt);
}

std::shared_ptr<TaskPool> Registry::root_task_pool()
{
    std::lock_guard<std::recursive_mutex> const lock(_taskPoolMtx);
    return _rootTaskPool;
}
#pragma endregion

#pragma region private
bool Registry::register_logger_it(std::shared_ptr<Logger> logger)
{
    if (exist_it(logger->name())) {
        ORIGIN_DEBUG_ERR("Logger already exist. LoggerName: {}.", logger->name());
        return false;
    }
    _loggers[logger->name()] = std::move(logger);
    return true;
}

void Registry::register_or_replace_logger_it(std::shared_ptr<Logger> logger)
{
    _loggers[logger->name()] = std::move(logger);
}

bool Registry::exist_it(std::string_view name) const
{
    return (_loggers.contains(name));
}
#pragma endregion
}  // namespace origin::logging
