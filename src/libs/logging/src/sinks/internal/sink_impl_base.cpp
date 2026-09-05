#include "sinks/internal/sink_impl_base.h"

#include <atomic>
#include <mutex>
#include <utility>

#include "common/debug/debug_logger.h"
#include "internal/common.h"
#include "logging/formatters/pattern_formatter.h"

namespace origin::logging {

SinkImplBase::SinkImplBase(std::string_view parameter) : _paramStr(parameter) {}

void SinkImplBase::log(const LogMsg& logMsg)
{
    std::lock_guard lock(_sinkMtx);
    try {
        log_it(logMsg);
    } catch (std::exception& ex) {
        ORIGIN_DEBUG_ERR(
            "Sink log failed. [Param]: \"{}\". [Exception]: \"{}\".", _paramStr, ex.what());
    }
}

void SinkImplBase::flush()
{
    std::lock_guard lock(_sinkMtx);
    try {
        flush_it();
    } catch (std::exception& ex) {
        ORIGIN_DEBUG_ERR(
            "Sink flush failed. [Param]: \"{}\". [Exception]: \"{}\".", _paramStr, ex.what());
    }
}

bool SinkImplBase::should_log(LogLevel level) const
{
    if (level == LogLevel::OFF) {
        return false;
    }
    return level >= _level.load(std::memory_order_relaxed);
}

void SinkImplBase::set_level(LogLevel level)
{
    _level.store(level, std::memory_order_relaxed);
};

LogLevel SinkImplBase::level() const
{
    return _level.load(std::memory_order_relaxed);
}

void SinkImplBase::set_pattern(std::string_view pattern)
{
    set_formatter(std::make_unique<PatternFormatter>(pattern));
}

void SinkImplBase::set_formatter(std::unique_ptr<Formatter> formatter)
{
    RETURN_IF_PTR_NULL(formatter);
    std::lock_guard const lock(_sinkMtx);
    _formatter = std::move(formatter);
}

std::string_view SinkImplBase::param_str() const
{
    return _paramStr;
}

}  // namespace origin::logging
