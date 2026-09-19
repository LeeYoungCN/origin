#include "sinks/internal/sink_impl_base.hpp"

#include <atomic>
#include <exception>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

#include "common/debug/debug_logger.h"
#include "internal/common.hpp"
#include "logging/formatters/formatter.hpp"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"

namespace origin::logging {

SinkImplBase::SinkImplBase(const std::string_view paramString) : _paramStr(paramString) {}

void SinkImplBase::log(const LogMsg& logMsg)
{
    std::lock_guard const lock(_sinkMtx);
    try {
        log_it(logMsg);
    } catch (std::exception& ex) {
        ORIGIN_DEBUG_ERR(
            "Sink log failed. [Param]: \"{}\". [Exception]: \"{}\".", _paramStr, ex.what());
    }
}

void SinkImplBase::flush()
{
    std::lock_guard const lock(_sinkMtx);
    try {
        flush_it();
    } catch (std::exception& ex) {
        ORIGIN_DEBUG_ERR(
            "Sink flush failed. [Param]: \"{}\". [Exception]: \"{}\".", _paramStr, ex.what());
    }
}

bool SinkImplBase::should_log(const LogLevel level) const
{
    if (level == LogLevel::OFF) {
        return false;
    }
    return level >= _level.load(std::memory_order_relaxed);
}

void SinkImplBase::set_level(const LogLevel level)
{
    _level.store(level, std::memory_order_relaxed);
};

LogLevel SinkImplBase::level() const
{
    return _level.load(std::memory_order_relaxed);
}

void SinkImplBase::set_pattern(const std::string_view pattern)
{
    try {
        set_formatter(std::make_unique<PatternFormatter>(pattern));
    } catch (std::exception& e) {
        ORIGIN_DEBUG_ERR("Sink set pattern failed. [Exception]: {}", e.what());
    }
}

void SinkImplBase::set_formatter(std::unique_ptr<Formatter> formatter)
{
    RETURN_AND_LOG_IF_PTR_NULL(formatter, "Sink set formatter failed.");
    std::lock_guard const lock(_sinkMtx);
    _formatter = std::move(formatter);
}

std::string_view SinkImplBase::param_string() const
{
    return _paramStr;
}

void SinkImplBase::set_param_string(const std::string_view param)
{
    _paramStr = param;
}

}  // namespace origin::logging
