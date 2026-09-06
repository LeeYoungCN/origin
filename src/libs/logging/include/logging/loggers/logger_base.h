#ifndef ORIGIN_LOGGING_LOGGERS_LOGGER_BASE_H
#define ORIGIN_LOGGING_LOGGERS_LOGGER_BASE_H
#include <memory>
#include <string_view>
#include <vector>

#include "logging/formatters/formatter.h"
#include "logging/log_level.h"
#include "logging/log_source.h"
#include "logging/loggers/logger.h"
#include "logging/sinks/sink.h"

#if COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace origin::logging {
class LOGGING_API LoggerBase : public Logger {
public:
    LoggerBase() = delete;
    explicit LoggerBase(std::shared_ptr<Logger> pImpl);
    ~LoggerBase() override;

    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] const std::vector<std::shared_ptr<Sink>>& sinks() const override;

    void set_level(LogLevel level) override;
    [[nodiscard]] LogLevel level() const override;
    [[nodiscard]] bool should_log(LogLevel level) const override;

    void flush_on(LogLevel level) override;
    [[nodiscard]] LogLevel flush_level() const override;
    [[nodiscard]] bool should_flush(LogLevel level) const override;

    void set_pattern(std::string_view pattern) const override;
    void set_formatter(const std::unique_ptr<Formatter>& formatter) const override;

    void flush() override;

    void force_log(const LogSource& source, LogLevel level, std::string_view message) override;

protected:
    void throw_if_pimpl_null() const;

    std::shared_ptr<Logger> _pImpl;
};
}  // namespace origin::logging

#if COMPILER_MSVC
#pragma warning(pop)
#endif
#endif  // ORIGIN_LOGGING_LOGGERS_LOGGER_BASE_H
