#ifndef ORIGIN_LOGGING_LOGGING_H
#define ORIGIN_LOGGING_LOGGING_H

#include <format>
#include <memory>
#include <string_view>
#include <utility>

#include "logging/formatters/formatter.h"
#include "logging/log_level.h"
#include "logging/log_source.h"
#include "logging/loggers/async_logger.h"
#include "logging/loggers/logger.h"
#include "logging/logging_api.h"
#include "logging/sinks/sink.h"

namespace origin::logging {
class TaskPool;

template <typename LoggerType, typename SinkType, typename... SinkArgs>
std::shared_ptr<LoggerType> create_logger(std::string_view name, SinkArgs&&... sinkArgs)
{
    return std::make_shared<LoggerType>(
        name, std::make_shared<SinkType>(std::forward<SinkArgs>(sinkArgs)...));
}

template <typename LoggerType>
std::shared_ptr<LoggerType> create_logger(std::string_view name, std::shared_ptr<Sink> sink)
{
    return std::make_shared<LoggerType>(name, std::move(sink));
}

template <typename LoggerType>
std::shared_ptr<LoggerType> create_logger(std::string_view name,
                                          std::initializer_list<std::shared_ptr<Sink>> sinks)
{
    return std::make_shared<LoggerType>(name, sinks);
}

std::shared_ptr<Logger> create_async_logger(std::string_view name,
                                            const std::shared_ptr<Sink>& sink,
                                            const std::weak_ptr<TaskPool>& pool);

std::shared_ptr<Logger> create_async_logger(std::string_view name,
                                            std::initializer_list<std::shared_ptr<Sink>> sinks,
                                            const std::weak_ptr<TaskPool>& pool);

template <typename SinkType, typename... SinkArgs>
std::shared_ptr<SinkType> create_sink(SinkArgs&&... sinkArgs)
{
    return std::make_shared<SinkType>(std::forward<SinkArgs>(sinkArgs)...);
}

template <typename FormatterType, typename... FormatterArgs>
std::unique_ptr<Formatter> create_formatter(FormatterArgs&&... formatterArgs)
{
    return std::make_unique<FormatterType>(std::forward<FormatterArgs>(formatterArgs)...);
}

std::shared_ptr<TaskPool> create_task_pool(uint32_t capacity, uint32_t threadCnt);

#pragma region Root logger
LOGGING_API std::shared_ptr<Logger> root_logger();
LOGGING_API Logger* root_logger_raw();
LOGGING_API void set_root_logger(std::shared_ptr<Logger> logger);

LOGGING_API bool should_log(LogLevel level);
LOGGING_API void set_level(LogLevel level);
LOGGING_API void flush_on(LogLevel level);
LOGGING_API void set_pattern(std::string_view pattern);
LOGGING_API void set_formatter(const std::unique_ptr<Formatter>& formatter);
LOGGING_API void flush();

template <typename... Args>
void log(LogLevel level, std::format_string<Args...> format, Args&&... args)
{
    root_logger()->log(level, std::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
void log(const LogSource& source, LogLevel level, std::format_string<Args...> format,
         Args&&... args)
{
    root_logger()->log(source, level, format, std::forward<Args>(args)...);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void log(const LogSource& source, LogLevel level, const T& msg)
{
    root_logger()->log(source, level, msg);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void log(LogLevel level, const T& msg)
{
    root_logger()->log(level, origin::string::type_to_string(msg));
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void trace(const LogSource& source, LogLevel level, const T& message)
{
    root_logger()->trace(source, level, message);
}

template <typename... Args>
void trace(std::format_string<Args...> format, Args&&... args)
{
    root_logger()->trace(format, std::forward<Args>(args)...);
}

template <typename... Args>
void trace(const LogSource& source, std::format_string<Args...> format, Args&&... args)
{
    root_logger()->trace(source, format, std::forward<Args>(args)...);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void trace(const T& message)
{
    root_logger()->trace(message);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void trace(const LogSource& source, const T& message)
{
    root_logger()->trace(source, message);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void debug(const LogSource& source, LogLevel level, const T& message)
{
    root_logger()->debug(source, level, message);
}

template <typename... Args>
void debug(std::format_string<Args...> format, Args&&... args)
{
    root_logger()->debug(format, std::forward<Args>(args)...);
}

template <typename... Args>
void debug(const LogSource& source, std::format_string<Args...> format, Args&&... args)
{
    root_logger()->debug(source, format, std::forward<Args>(args)...);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void debug(const T& message)
{
    root_logger()->debug(message);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void debug(const LogSource& source, const T& message)
{
    root_logger()->debug(source, message);
}

template <typename... Args>
void info(std::format_string<Args...> format, Args&&... args)
{
    root_logger()->info(format, std::forward<Args>(args)...);
}

template <typename... Args>
void info(const LogSource& source, std::format_string<Args...> format, Args&&... args)
{
    root_logger()->info(source, format, std::forward<Args>(args)...);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void info(const T& message)
{
    root_logger()->info(message);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void info(const LogSource& source, const T& message)
{
    root_logger()->info(source, message);
}

template <typename... Args>
void warn(std::format_string<Args...> format, Args&&... args)
{
    root_logger()->warn(format, std::forward<Args>(args)...);
}

template <typename... Args>
void warn(const LogSource& source, std::format_string<Args...> format, Args&&... args)
{
    root_logger()->warn(source, format, std::forward<Args>(args)...);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void warn(const T& message)
{
    root_logger()->warn(message);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void warn(const LogSource& source, const T& message)
{
    root_logger()->warn(source, message);
}

template <typename... Args>
void error(std::format_string<Args...> format, Args&&... args)
{
    root_logger()->error(format, std::forward<Args>(args)...);
}

template <typename... Args>
void error(const LogSource& source, std::format_string<Args...> format, Args&&... args)
{
    root_logger()->error(source, format, std::forward<Args>(args)...);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void error(const T& message)
{
    root_logger()->error(message);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void error(const LogSource& source, const T& message)
{
    root_logger()->error(source, message);
}

template <typename... Args>
void fatal(std::format_string<Args...> format, Args&&... args)
{
    root_logger()->fatal(format, std::forward<Args>(args)...);
}

template <typename... Args>
void fatal(const LogSource& source, std::format_string<Args...> format, Args&&... args)
{
    root_logger()->fatal(source, format, std::forward<Args>(args)...);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void fatal(const T& message)
{
    root_logger()->fatal(message);
}

template <class T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
void fatal(const LogSource& source, const T& message)
{
    root_logger()->fatal(source, message);
}
#pragma endregion

#pragma region logging manager
LOGGING_API void initialize_logger(const std::shared_ptr<Logger>& logger);
LOGGING_API void set_level_all(LogLevel level);
LOGGING_API void flush_on_all(LogLevel level);
LOGGING_API void set_pattern_all(std::string_view pattern);
LOGGING_API void set_formatter_all(std::unique_ptr<Formatter> formatter);
LOGGING_API void flush_all();
LOGGING_API void shutdown();
#pragma endregion

#pragma region registry
LOGGING_API bool register_logger(std::shared_ptr<Logger> logger);
LOGGING_API void register_or_replace_logger(std::shared_ptr<Logger> logger);
LOGGING_API void remove_logger(std::string_view name);
LOGGING_API void remove_all();
LOGGING_API std::shared_ptr<Logger> get_logger(std::string_view name);

LOGGING_API void init_root_task_pool(uint32_t capacity, uint32_t threadCnt);
LOGGING_API std::shared_ptr<TaskPool> root_task_pool();
#pragma endregion

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGING_H
