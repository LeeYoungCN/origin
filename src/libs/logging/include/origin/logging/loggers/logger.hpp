#ifndef ORIGIN_LOGGING_LOGGERS_LOGGER_HPP
#define ORIGIN_LOGGING_LOGGERS_LOGGER_HPP

#include <format>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "origin/logging/detail/logging_api.h"
#include "origin/logging/formatters/formatter.hpp"
#include "origin/logging/log_level.hpp"
#include "origin/logging/log_source.hpp"
#include "origin/logging/sinks/sink.hpp"

namespace origin::logging {
class LoggerBase;
class LOGGING_API Logger {
public:
    Logger() = default;
    virtual ~Logger() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;
    [[nodiscard]] virtual const std::vector<std::shared_ptr<Sink>>& sinks() const = 0;

    virtual void set_level(LogLevel level) = 0;
    [[nodiscard]] virtual LogLevel level() const = 0;
    [[nodiscard]] virtual bool should_log(LogLevel level) const = 0;

    virtual void flush_on(LogLevel level) = 0;
    [[nodiscard]] virtual LogLevel flush_level() const = 0;
    [[nodiscard]] virtual bool should_flush(LogLevel level) const = 0;

    virtual void set_pattern(std::string_view pattern) const = 0;
    virtual void set_formatter(const std::unique_ptr<Formatter>& formatter) const = 0;

    virtual void flush() = 0;

#pragma region log function
    void log(const LogSource& source, const LogLevel level, const std::string_view message)
    {
        if (should_log(level)) {
            log_unchecked(source, level, message);
        }
    }

    void log(const LogLevel level, const std::string_view message)
    {
        log(LogSource(), level, message);
    }

    template <typename... Args>
    void log(const LogLevel level, std::format_string<Args...> format, Args&&... args)
    {
        log(LogSource(), level, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log(const LogSource& source,
             const LogLevel level,
             std::format_string<Args...> format,
             Args&&... args)
    {
        if (should_log(level)) {
            log_unchecked(source, level, std::format(format, std::forward<Args>(args)...));
        }
    }

    template <typename... Args>
    void trace(const LogSource& source, std::format_string<Args...> format, Args&&... args)
    {
        log(source, LogLevel::TRACE, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(const LogSource& source, std::format_string<Args...> format, Args&&... args)
    {
        log(source, LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(const LogSource& source, std::format_string<Args...> format, Args&&... args)
    {
        log(source, LogLevel::INFO, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(const LogSource& source, std::format_string<Args...> format, Args&&... args)
    {
        log(source, LogLevel::WARN, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(const LogSource& source, std::format_string<Args...> format, Args&&... args)
    {
        log(source, LogLevel::ERR, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void fatal(const LogSource& source, std::format_string<Args...> format, Args&&... args)
    {
        log(source, LogLevel::FATAL, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void trace(std::format_string<Args...> format, Args&&... args)
    {
        log(LogSource(), LogLevel::TRACE, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(std::format_string<Args...> format, Args&&... args)
    {
        log(LogSource(), LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(std::format_string<Args...> format, Args&&... args)
    {
        log(LogSource(), LogLevel::INFO, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(std::format_string<Args...> format, Args&&... args)
    {
        log(LogSource(), LogLevel::WARN, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(std::format_string<Args...> format, Args&&... args)
    {
        log(LogSource(), LogLevel::ERR, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void fatal(std::format_string<Args...> format, Args&&... args)
    {
        log(LogSource(), LogLevel::FATAL, format, std::forward<Args>(args)...);
    }

    void trace(const std::string_view message) { log(LogSource(), LogLevel::TRACE, message); }

    void debug(const std::string_view message) { log(LogSource(), LogLevel::DEBUG, message); }

    void info(const std::string_view message) { log(LogSource(), LogLevel::INFO, message); }

    void warn(const std::string_view message) { log(LogSource(), LogLevel::WARN, message); }

    void error(const std::string_view message) { log(LogSource(), LogLevel::ERR, message); }

    void fatal(const std::string_view message) { log(LogSource(), LogLevel::FATAL, message); }

    void trace(const LogSource& source, const std::string_view message)
    {
        log(source, LogLevel::TRACE, message);
    }

    void debug(const LogSource& source, const std::string_view message)
    {
        log(source, LogLevel::DEBUG, message);
    }

    void info(const LogSource& source, const std::string_view message)
    {
        log(source, LogLevel::INFO, message);
    }

    void warn(const LogSource& source, const std::string_view message)
    {
        log(source, LogLevel::WARN, message);
    }

    void error(const LogSource& source, const std::string_view message)
    {
        log(source, LogLevel::ERR, message);
    }

    void fatal(const LogSource& source, const std::string_view message)
    {
        log(source, LogLevel::FATAL, message);
    }

#pragma endregion

protected:
    virtual void log_unchecked(const LogSource& source,
                               LogLevel level,
                               std::string_view message) = 0;
    friend class LoggerBase;
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGERS_LOGGER_HPP
