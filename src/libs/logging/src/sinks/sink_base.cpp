#include "logging/sinks/sink_base.h"

#include <stdexcept>
#include <utility>

#include "common/debug/debug_logger.h"

namespace origin::logging {
SinkBase::~SinkBase()
{
    if (_pImpl == nullptr) {
        return;
    }
    ORIGIN_DEBUG_DBG("Release Sink. {}", _pImpl->param_str());
    _pImpl.reset();
}

SinkBase::SinkBase(std::unique_ptr<Sink> pImpl) : _pImpl(std::move(pImpl))
{
    throw_if_pimpl_null();
    ORIGIN_DEBUG_DBG("Create Sink. {}", _pImpl->param_str());
}

void SinkBase::log(const LogMsg& logMsg)
{
    throw_if_pimpl_null();
    _pImpl->log(logMsg);
}

void SinkBase::flush()
{
    throw_if_pimpl_null();
    _pImpl->flush();
}

void SinkBase::set_pattern(std::string_view pattern)
{
    throw_if_pimpl_null();
    _pImpl->set_pattern(pattern);
}

void SinkBase::set_formatter(std::unique_ptr<Formatter> formatter)
{
    throw_if_pimpl_null();
    _pImpl->set_formatter(std::move(formatter));
}

bool SinkBase::should_log(LogLevel level) const
{
    throw_if_pimpl_null();
    return _pImpl->should_log(level);
}

void SinkBase::set_level(LogLevel level)
{
    throw_if_pimpl_null();
    return _pImpl->set_level(level);
}

LogLevel SinkBase::level() const
{
    throw_if_pimpl_null();
    return _pImpl->level();
}

std::string_view SinkBase::param_str() const
{
    throw_if_pimpl_null();
    return _pImpl->param_str();
}

void SinkBase::throw_if_pimpl_null() const
{
    if (_pImpl == nullptr) {
        throw std::runtime_error("pImpl nullptr.");
    }
}

}  // namespace origin::logging
