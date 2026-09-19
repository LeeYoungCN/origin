#include "loggers/internal/logger_impl_base.hpp"

#include <atomic>
#include <exception>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "common/debug/debug_logger.h"
#include "logging/formatters/formatter.hpp"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/log_msg.hpp"
#include "logging/log_source.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {
LoggerImplBase::LoggerImplBase(const std::string_view name, const std::shared_ptr<Sink>& sink)
    : _name(name), _sinks({sink})
{
    throw_if_param_invalid();
}

void LoggerImplBase::throw_if_param_invalid()
{
    if (_name.empty()) {
        constexpr auto LOGGER_NAME_EMPTY = "Name cannot be empty.";
        ORIGIN_DEBUG_ERR("Create logger failed. {}", LOGGER_NAME_EMPTY);
        throw std::invalid_argument(LOGGER_NAME_EMPTY);
    }

    if (_sinks.empty()) {
        constexpr auto SINKS_EMPTY = "Sinks cannot be empty.";
        ORIGIN_DEBUG_ERR("Create logger failed. Name: [{}]. {}", _name, SINKS_EMPTY);
        throw std::invalid_argument(SINKS_EMPTY);
    }

    for (const auto& sink : _sinks) {
        if (sink == nullptr) {
            constexpr auto SINK_NULL = "Sink cannot be nullptr.";
            ORIGIN_DEBUG_ERR("Create logger failed. Name: [{}]. {}", _name, SINK_NULL);
            throw std::invalid_argument(SINK_NULL);
        }
    }
}

std::string_view LoggerImplBase::name() const
{
    return _name;
}

const std::vector<std::shared_ptr<Sink>>& LoggerImplBase::sinks() const
{
    return _sinks;
}

void LoggerImplBase::set_level(LogLevel level)
{
    _level.store(level, std::memory_order_relaxed);
}

LogLevel LoggerImplBase::level() const
{
    return _level.load(std::memory_order_relaxed);
}

bool LoggerImplBase::should_log(const LogLevel level) const
{
    return (level != LogLevel::OFF && level >= this->level());
}

void LoggerImplBase::flush_on(const LogLevel level)
{
    _flushLevel.store(level, std::memory_order_relaxed);
}

LogLevel LoggerImplBase::flush_level() const
{
    return _flushLevel.load(std::memory_order_relaxed);
}

bool LoggerImplBase::should_flush(const LogLevel level) const
{
    return (level != LogLevel::OFF && level >= this->flush_level());
}

void LoggerImplBase::set_pattern(const std::string_view pattern) const
{
    try {
        set_formatter(std::make_unique<PatternFormatter>(pattern));
    } catch (std::exception& e) {
        ORIGIN_DEBUG_ERR("Logger set pattern failed. Name: [{}]. {}", _name, e.what());
    }
}

void LoggerImplBase::set_formatter(const std::unique_ptr<Formatter>& formatter) const
{
    if (formatter == nullptr) {
        ORIGIN_DEBUG_ERR("Logger set formatter failed. Name: [{}]. formatter nullptr.", _name);
        return;
    }
    for (auto& sink : _sinks) {
        sink->set_formatter(formatter->clone());
    }
}

void LoggerImplBase::force_log(const LogSource& source, const LogLevel level,
                               const std::string_view message)
{
    log_it(create_log_msg(source, name(), level, message));
}

void LoggerImplBase::flush()
{
    flush_it();
}

void LoggerImplBase::backend_log(const LogMsg& logMsg) const
{
    for (const auto& sink : _sinks) {
        if (sink->should_log(logMsg.level)) {
            sink->log(logMsg);
        }
    }
    if (should_flush(logMsg.level)) {
        backend_flush();
    }
}

void LoggerImplBase::backend_flush() const
{
    for (const auto& sink : _sinks) {
        sink->flush();
    }
}

}  // namespace origin::logging
