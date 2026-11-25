#include <cstdint>
#include <memory>

#include "origin/common/debug/debug_logger.h"

#include "origin/logging/c/logging_c.h"
#include "origin/logging/sinks/basic_file_sink.hpp"
#include "origin/logging/sinks/daily_file_sink.hpp"
#include "origin/logging/sinks/rotating_file_sink.hpp"
#include "origin/logging/sinks/stderr_sink.hpp"
#include "origin/logging/sinks/stdout_sink.hpp"

#include "c/internal/common_c.hpp"

using namespace origin::logging;
using namespace origin::logging::c_api;

extern "C" {
SinkSt* origin_create_stdout_sink()
{
    try {
        return new SinkSt(std::make_shared<StdoutSink>());
    } catch (...) {
        return nullptr;
    }
}

SinkSt* origin_create_stderr_sink()
{
    try {
        return new SinkSt(std::make_shared<StderrSink>());
    } catch (...) {
        return nullptr;
    }
}

SinkSt* origin_create_basic_file_sink(const char* file, const bool overwrite)
{
    try {
        return new SinkSt(std::make_shared<BasicFileSink>(file, overwrite));
    } catch (...) {
        return nullptr;
    }
}

SinkSt* origin_create_daily_file_sink(const char* file,
                                      const uint32_t hour,
                                      const uint32_t minute,
                                      const uint32_t maxFiles,
                                      const bool overwrite)
{
    try {
        return new SinkSt(std::make_shared<DailyFileSink>(file, hour, minute, maxFiles, overwrite));
    } catch (...) {
        return nullptr;
    }
}

SinkSt* origin_create_rotating_file_sink(const char* file,
                                         const uint32_t maxFileSize,
                                         const uint32_t maxFiles,
                                         const bool rotateOnOpen)
{
    try {
        return new SinkSt(
            std::make_shared<RotatingFileSink>(file, maxFileSize, maxFiles, rotateOnOpen));
    } catch (...) {
        return nullptr;
    }
}

void origin_destroy_sink(SinkSt* sink)
{
    if (sink == nullptr) {
        return;
    }
    try {
        if (sink->ptr != nullptr) {
            ORIGIN_DEBUG_DBG(
                "Release SinkSt. UseCnt: {}. {}", sink->ptr.use_count(), sink->ptr->param_string());
            sink->ptr.reset();
        }
    } catch (...) {
        // C ABI 边界不允许异常穿出；析构仍需继续。
    }
    delete sink;
}

void origin_sink_set_level(const SinkSt* sink, const LogLevelC level)
{
    try {
        if (PTR_INVALID(sink)) {
            ORIGIN_DEBUG_ERR("Sink set level failed. {}", SINK_NULL_LOG);
            return;
        }
        if (log_level_c_invalid(level)) {
            ORIGIN_DEBUG_ERR("Sink set level failed. param: [{}]. {}",
                             sink->ptr->param_string(),
                             LOG_LEVEL_C_INVALID_LOG);
            return;
        }
        sink->ptr->set_level(c_to_cpp_log_level(level));
    } catch (...) {
    }
}

bool origin_sink_should_log(const SinkSt* sink, const LogLevelC level)
{
    try {
        if (PTR_INVALID(sink)) {
            ORIGIN_DEBUG_WARN("Sink should log failed. {}", SINK_NULL_LOG);
            return false;
        }
        if (log_level_c_invalid(level)) {
            ORIGIN_DEBUG_WARN("Sink should log failed. param: [{}]. {}",
                              sink->ptr->param_string(),
                              LOG_LEVEL_C_INVALID_LOG);
            return false;
        }
        return sink->ptr->should_log(c_to_cpp_log_level(level));
    } catch (...) {
        return false;
    }
}

LogLevelC origin_sink_level(const SinkSt* sink)
{
    try {
        if (PTR_INVALID(sink)) {
            ORIGIN_DEBUG_ERR("Sink get level failed. {}", SINK_NULL_LOG);
            return LOG_LEVEL_C_OFF;
        }
        return cpp_to_c_log_level(sink->ptr->level());
    } catch (...) {
        return LOG_LEVEL_C_OFF;
    }
}

void origin_sink_set_pattern(const SinkSt* sink, const char* pattern)
{
    try {
        if (PTR_INVALID(sink)) {
            ORIGIN_DEBUG_ERR("Sink set pattern failed. {}", SINK_NULL_LOG);
            return;
        }
        if (pattern == nullptr) {
            ORIGIN_DEBUG_ERR("Sink set pattern failed. pattern nullptr.");
            return;
        }
        sink->ptr->set_pattern(pattern);
    } catch (...) {
    }
}

void origin_sink_set_formatter(const SinkSt* sink, const FormatterSt* formatter)
{
    try {
        if (PTR_INVALID(sink)) {
            ORIGIN_DEBUG_ERR("Sink set formatter failed. {}", SINK_NULL_LOG);
            return;
        }
        if (PTR_INVALID(formatter)) {
            ORIGIN_DEBUG_ERR("Sink set formatter failed. {}", FORMATTER_NULL_LOG);
            return;
        }
        sink->ptr->set_formatter(formatter->ptr->clone());
    } catch (...) {
    }
}
}
