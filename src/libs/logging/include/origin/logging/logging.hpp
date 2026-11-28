#ifndef ORIGIN_LOGGING_LOGGING_HPP
#define ORIGIN_LOGGING_LOGGING_HPP

#include <cstdint>
#include <format>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "origin/logging/detail/logging_api.h"
#include "origin/logging/formatters/formatter.hpp"
#include "origin/logging/log_level.hpp"
#include "origin/logging/log_source.hpp"
#include "origin/logging/loggers/async_logger.hpp"
#include "origin/logging/loggers/logger.hpp"
#include "origin/logging/sinks/sink.hpp"

namespace origin::logging {
class TaskPool;

template <typename LoggerType, typename SinkType, typename... SinkArgs>
std::shared_ptr<LoggerType> create_logger(std::string_view name, SinkArgs&&... sinkArgs)
{
    return std::make_shared<LoggerType>(
        name, std::make_shared<SinkType>(std::forward<SinkArgs>(sinkArgs)...));
}

template <typename LoggerType>
std::shared_ptr<LoggerType> create_logger(std::string_view name, const std::shared_ptr<Sink>& sink)
{
    return std::make_shared<LoggerType>(name, sink);
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

LOGGING_API std::shared_ptr<TaskPool> create_task_pool(uint32_t capacity, uint32_t threadCnt);

#pragma region Root logger
LOGGING_API std::shared_ptr<Logger> root_logger();
LOGGING_API void set_root_logger(std::shared_ptr<Logger> logger);

LOGGING_API void set_level(LogLevel level);
LOGGING_API LogLevel level();
LOGGING_API bool should_log(LogLevel level);

LOGGING_API void flush_on(LogLevel level);
LOGGING_API LogLevel flush_level();
LOGGING_API bool should_flush(LogLevel level);

LOGGING_API void set_pattern(std::string_view pattern);
LOGGING_API void set_formatter(const std::unique_ptr<Formatter>& formatter);
LOGGING_API void flush();

// 非模板便捷函数直接定义在头文件，必须 inline，否则每个包含该头文件的
// 翻译单元都会生成强符号，链接时出现 LNK2005。
inline void log(LogLevel level, std::string_view message)
{
    root_logger()->log(level, message);
}

inline void log(const LogSource& source, LogLevel level, std::string_view message)
{
    root_logger()->log(source, level, message);
}

template <typename... Args>
void log(LogLevel level, std::format_string<Args...> format, Args&&... args)
{
    root_logger()->log(level, format, std::forward<Args>(args)...);
}

template <typename... Args>
void log(const LogSource& source,
         LogLevel level,
         std::format_string<Args...> format,
         Args&&... args)
{
    root_logger()->log(source, level, format, std::forward<Args>(args)...);
}

inline void trace(std::string_view message)
{
    root_logger()->trace(message);
}

inline void trace(const LogSource& source, std::string_view message)
{
    root_logger()->trace(source, message);
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

inline void debug(std::string_view message)
{
    root_logger()->debug(message);
}

inline void debug(const LogSource& source, std::string_view message)
{
    root_logger()->debug(source, message);
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

inline void info(std::string_view message)
{
    root_logger()->info(message);
}

inline void info(const LogSource& source, std::string_view message)
{
    root_logger()->info(source, message);
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

inline void warn(std::string_view message)
{
    root_logger()->warn(message);
}

inline void warn(const LogSource& source, std::string_view message)
{
    root_logger()->warn(source, message);
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

inline void error(std::string_view message)
{
    root_logger()->error(message);
}

inline void error(const LogSource& source, std::string_view message)
{
    root_logger()->error(source, message);
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

inline void fatal(std::string_view message)
{
    root_logger()->fatal(message);
}

inline void fatal(const LogSource& source, std::string_view message)
{
    root_logger()->fatal(source, message);
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

#pragma endregion

#pragma region logging manager
LOGGING_API void initialize_logger(const std::shared_ptr<Logger>& logger);
LOGGING_API void set_level_all(LogLevel level);
LOGGING_API void flush_on_all(LogLevel level);
LOGGING_API void set_pattern_all(std::string_view pattern);
LOGGING_API void set_formatter_all(std::unique_ptr<Formatter> formatter);
/**
 * @brief 向所有已注册 logger 各写一条日志。
 *
 * 按每个 logger 自己的级别过滤，返回实际写入的 logger 数量。消息只格式化一次；
 * 若多个 logger 共享同一个 sink，该 sink 会收到多条（每个 logger 各一条）。
 */
LOGGING_API uint32_t log_all(const LogSource& source, LogLevel level, std::string_view message);

template <typename... Args>
uint32_t log_all(const LogSource& source,
                 LogLevel level,
                 std::format_string<Args...> format,
                 Args&&... args)
{
    const std::string text = std::format(format, std::forward<Args>(args)...);
    return log_all(source, level, std::string_view(text));
}

LOGGING_API void flush_all();
LOGGING_API void apply_all(const std::function<void(const std::shared_ptr<Logger>&)>& fun);
LOGGING_API void shutdown();
#pragma endregion

#pragma region registry
LOGGING_API bool register_logger(std::shared_ptr<Logger> logger);
LOGGING_API void register_or_replace_logger(std::shared_ptr<Logger> logger);
LOGGING_API void remove_logger(std::string_view name);
LOGGING_API void remove_all();
LOGGING_API std::shared_ptr<Logger> get_logger(std::string_view name);
LOGGING_API bool is_logger_exists(std::string_view name);
LOGGING_API std::vector<std::shared_ptr<Logger>> snapshot_loggers();

LOGGING_API bool set_root_task_pool(std::shared_ptr<TaskPool> taskPool);
LOGGING_API std::shared_ptr<TaskPool> root_task_pool();
#pragma endregion

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGING_HPP
