#include "detail/mock_sinks/base_test_sink.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

#include "common/debug/debug_logger.h"
#include "logging/formatters/formatter.hpp"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/log_msg.hpp"

namespace logging_test {
using namespace origin::logging;

BaseTestSink::~BaseTestSink()
{
    ORIGIN_DEBUG_DBG("Release Sink. {}", _paramStr);
}

BaseTestSink::BaseTestSink(const std::string_view parameter) : _paramStr(parameter) {}

void BaseTestSink::log(const LogMsg& logMsg)
{
    std::lock_guard const lock(_sinkMtx);
    log_it(logMsg);
}

void BaseTestSink::flush()
{
    std::lock_guard const lock(_sinkMtx);
    flush_it();
}

bool BaseTestSink::should_log(const LogLevel level) const
{
    if (level == LogLevel::OFF) {
        return false;
    }
    return level >= _level.load(std::memory_order_relaxed);
}

void BaseTestSink::set_level(const LogLevel level)
{
    _level.store(level, std::memory_order_relaxed);
};

LogLevel BaseTestSink::level() const
{
    return _level.load(std::memory_order_relaxed);
}

void BaseTestSink::set_pattern(const std::string_view pattern)
{
    set_formatter(std::make_unique<PatternFormatter>(pattern));
}

void BaseTestSink::set_formatter(std::unique_ptr<Formatter> formatter)
{
    std::lock_guard const lock(_sinkMtx);
    _formatter = std::move(formatter);
}

std::string_view BaseTestSink::param_string() const
{
    return _paramStr;
}

}  // namespace logging_test
