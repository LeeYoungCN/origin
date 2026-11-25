#include "sinks/internal/sink_impl_base.hpp"

#include <atomic>
#include <exception>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

#include "origin/common/debug/debug_logger.h"

#include "origin/logging/formatters/formatter.hpp"
#include "origin/logging/formatters/pattern_formatter.hpp"
#include "origin/logging/log_level.hpp"

namespace origin::logging {

SinkImplBase::SinkImplBase(const std::string_view paramString) : _paramStr(paramString) {}

void SinkImplBase::log(const LogMsg& logMsg)
{
    std::lock_guard const lock(_sinkMtx);
    try {
        log_it(logMsg);
    } catch (std::exception& ex) {
        report_error("log", ex.what());
    } catch (...) {
        report_error("log", "unknown exception");
    }
}

void SinkImplBase::flush()
{
    std::lock_guard const lock(_sinkMtx);
    try {
        flush_it();
    } catch (std::exception& ex) {
        report_error("flush", ex.what());
    } catch (...) {
        report_error("flush", "unknown exception");
    }
}

void SinkImplBase::report_error(const char* action, const char* detail) const noexcept
{
    try {
        std::string message = "Sink ";
        message += action;
        message += " failed. [Param]: \"";
        message += _paramStr;
        message += "\". [Exception]: \"";
        message += detail == nullptr ? "unknown" : detail;
        message += "\".";
        origin_debug_logger_force_log(__FILE__, __LINE__, __FUNCTION__, ORG_DBG_LVL_ERR, message);
    } catch (...) {
        // 错误上报本身不能再抛异常。
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
    set_formatter(std::make_unique<PatternFormatter>(pattern));
}

void SinkImplBase::set_formatter(std::unique_ptr<Formatter> formatter)
{
    if (!formatter) {
        ORIGIN_DEBUG_ERR("Sink set formatter failed.");
        return;
    }
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
