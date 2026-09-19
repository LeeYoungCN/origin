#include "logging/loggers/logger_base.hpp"

#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "common/debug/debug_logger.h"
#include "logging/formatters/formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/log_source.hpp"
#include "logging/loggers/logger.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {
LoggerBase::LoggerBase(std::shared_ptr<Logger> pImpl) : _pImpl(std::move(pImpl))
{
    throw_if_pimpl_null();
    ORIGIN_DEBUG_DBG("Create logger. Name: \"{}\".", _pImpl->name());
}

LoggerBase::~LoggerBase()
{
    if (_pImpl != nullptr) {
        ORIGIN_DEBUG_DBG("Release logger. Name: \"{}\".", _pImpl->name());
        _pImpl.reset();
    }
}

std::string_view LoggerBase::name() const
{
    throw_if_pimpl_null();
    return _pImpl->name();
}

const std::vector<std::shared_ptr<Sink>>& LoggerBase::sinks() const
{
    throw_if_pimpl_null();
    return _pImpl->sinks();
}

void LoggerBase::set_level(const LogLevel level)
{
    throw_if_pimpl_null();
    _pImpl->set_level(level);
}

LogLevel LoggerBase::level() const
{
    throw_if_pimpl_null();
    return _pImpl->level();
}
bool LoggerBase::should_log(const LogLevel level) const
{
    throw_if_pimpl_null();
    return _pImpl->should_log(level);
}

void LoggerBase::flush_on(const LogLevel level)
{
    throw_if_pimpl_null();
    _pImpl->flush_on(level);
}

LogLevel LoggerBase::flush_level() const
{
    throw_if_pimpl_null();
    return _pImpl->flush_level();
}

bool LoggerBase::should_flush(const LogLevel level) const
{
    throw_if_pimpl_null();
    return _pImpl->should_flush(level);
}

void LoggerBase::set_pattern(const std::string_view pattern) const
{
    throw_if_pimpl_null();
    _pImpl->set_pattern(pattern);
}

void LoggerBase::set_formatter(const std::unique_ptr<Formatter>& formatter) const
{
    throw_if_pimpl_null();
    _pImpl->set_formatter(formatter);
}

void LoggerBase::flush()
{
    throw_if_pimpl_null();
    _pImpl->flush();
}

void LoggerBase::force_log(const LogSource& source, const LogLevel level,
                           const std::string_view message)
{
    throw_if_pimpl_null();
    _pImpl->force_log(source, level, message);
}

void LoggerBase::throw_if_pimpl_null() const
{
    if (_pImpl == nullptr) {
        throw std::runtime_error("pImpl nullptr.");
    }
}

}  // namespace origin::logging
