#ifndef ORIGIN_LOGGING_LOGGERS_LOGGER_H
#define ORIGIN_LOGGING_LOGGERS_LOGGER_H

#include <format>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "common/types/type_traits.h"
#include "logging/formatters/formatter.h"
#include "logging/log_level.h"
#include "logging/log_source.h"
#include "logging/logging_api.h"
#include "logging/sinks/sink.h"
#include "utils/string_utils.h"

namespace origin::logging {
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

    virtual void force_log(const LogSource& source, LogLevel level, std::string_view message) = 0;
    virtual void flush() = 0;

#pragma region log function
    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void log(const LogSource& source, LogLevel level, const T& message)
    {
        if (should_log(level)) {
            force_log(source, level, origin::string::type_to_string(message));
        }
    }

    template <typename... Args>
    void log(const LogSource& source, LogLevel level, std::format_string<Args...> format,
             Args&&... args)
    {
        if (should_log(level)) {
            force_log(source, level, std::format(format, std::forward<Args>(args)...));
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

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void trace(const T& message)
    {
        log(LogSource(), LogLevel::TRACE, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void debug(const T& message)
    {
        log(LogSource(), LogLevel::DEBUG, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void info(const T& message)
    {
        log(LogSource(), LogLevel::INFO, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void warn(const T& message)
    {
        log(LogSource(), LogLevel::WARN, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void error(const T& message)
    {
        log(LogSource(), LogLevel::ERR, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void fatal(const T& message)
    {
        log(LogSource(), LogLevel::FATAL, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void trace(const LogSource& source, const T& message)
    {
        log(source, LogLevel::TRACE, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void debug(const LogSource& source, const T& message)
    {
        log(source, LogLevel::DEBUG, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void info(const LogSource& source, const T& message)
    {
        log(source, LogLevel::INFO, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void warn(const LogSource& source, const T& message)
    {
        log(source, LogLevel::WARN, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void error(const LogSource& source, const T& message)
    {
        log(source, LogLevel::ERR, message);
    }

    template <class T,
              std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
    void fatal(const LogSource& source, const T& message)
    {
        log(source, LogLevel::FATAL, message);
    }
#pragma endregion
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGERS_LOGGER_H
