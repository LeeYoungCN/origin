#include <cstdint>
#include <memory>

#include "c/internal/common_c.hpp"
#include "common/debug/debug_logger.h"
#include "internal/common.hpp"
#include "logging/c/logging_c.h"
#include "logging/sinks/basic_file_sink.hpp"
#include "logging/sinks/daily_file_sink.hpp"
#include "logging/sinks/stderr_sink.hpp"
#include "logging/sinks/stdout_sink.hpp"

using namespace origin::logging;
using namespace origin::logging::c_api;

extern "C" {
SinkSt *origin_create_stdout_sink()
{
    return new SinkSt(std::make_shared<StdoutSink>());
}

SinkSt *origin_create_stderr_sink()
{
    return new SinkSt(std::make_shared<StderrSink>());
}

SinkSt *origin_create_basic_file_sink(const char *file, const bool overwrite)
{
    return new SinkSt(std::make_shared<BasicFileSink>(file, overwrite));
}

SinkSt *origin_create_daily_file_sink(const char *file, const uint32_t hour, const uint32_t minute,
                                      const uint32_t maxFiles, const bool overwrite)
{
    return new SinkSt(std::make_shared<DailyFileSink>(file, hour, minute, maxFiles, overwrite));
}

SinkSt *origin_create_rotating_file_sink(const char *file, const uint32_t maxFileSize,
                                         const uint32_t maxFiles, const bool rotateOnOpen)
{
    return new SinkSt(std::make_shared<DailyFileSink>(file, maxFileSize, maxFiles, rotateOnOpen));
}

void origin_detroy_sink(SinkSt *sink)
{
    if (sink != nullptr) {
        if (sink->ptr != nullptr) {
            ORIGIN_DEBUG_DBG(
                "Release SinkSt. UseCnt: {}. {}", sink->ptr.use_count(), sink->ptr->param_string());
            sink->ptr.reset();
        }
        delete sink;
    }
}

void origin_sink_set_level(const SinkSt *sink, const LogLevelC level)
{
    RETURN_AND_LOG_IF_PTR_NULL(sink, "sink set level.");
    sink->ptr->set_level(c_to_cpp_log_level(level));
}

bool origin_sink_should_log(const SinkSt *sink, const LogLevelC level)
{
    RETURN_VALUE_IF_PTR_NULL(sink, false);
    return sink->ptr->should_log(c_to_cpp_log_level(level));
}

LogLevelC origin_sink_level(const SinkSt *sink)
{
    RETURN_VALUE_IF_PTR_NULL(sink, ORIGIN_LOG_LEVEL_OFF);
    return cpp_to_c_log_level(sink->ptr->level());
}

void origin_sink_set_pattern(const SinkSt *sink, const char *pattern)
{
    RETURN_AND_LOG_IF_PTR_NULL(sink, "sink set pattern.");
    RETURN_AND_LOG_IF_PTR_NULL(pattern, "sink set pattern.");
    sink->ptr->set_pattern(pattern);
}

void origin_sink_set_formatter(const SinkSt *sink, const FormatterSt *formatter)
{
    RETURN_AND_LOG_IF_PTR_NULL(sink, "Sink set formatter failed.");
    RETURN_AND_LOG_IF_PTR_NULL(formatter, "Sink set formatter failed.");
    RETURN_AND_LOG_IF_PTR_NULL(formatter->ptr, "Sink set formatter failed.");
    sink->ptr->set_formatter(formatter->ptr->clone());
}
}
