#include "c/common_c.h"
#include "common/debug/debug_logger.h"
#include "internal/common.h"
#include "logging/c/logging_c.h"
#include "logging/sinks/basic_file_sink.h"
#include "logging/sinks/daily_file_sink.h"
#include "logging/sinks/rotating_file_sink.h"
#include "logging/sinks/stdout_sink.h"

using namespace origin::logging;
using namespace origin::logging::c;

extern "C" {
SinkSt *origin_create_stdout_sink(FILE *file)
{
    if (file == nullptr) {
        return new struct SinkSt(std::make_shared<StdoutSink>());
    } else {
        return new struct SinkSt(std::make_shared<StdoutSink>(file));
    }
}

SinkSt *origin_create_basic_file_sink(const char *file, bool overwrite)
{
    if (file == nullptr) {
        return new struct SinkSt(std::make_shared<BasicFileSink>());
    } else {
        return new struct SinkSt(std::make_shared<BasicFileSink>(file, overwrite));
    }
}

SinkSt *origin_create_daily_file_sink(const char *file, uint32_t hour, uint32_t minute,
                                      uint32_t maxFiles, bool overwrite)
{
    if (file == nullptr) {
        return new struct SinkSt(std::make_shared<DailyFileSink>());
    } else {
        return new struct SinkSt(
            std::make_shared<DailyFileSink>(file, hour, minute, maxFiles, overwrite));
    }
}

SinkSt *origin_create_rotating_file_sink(const char *file, uint32_t maxFileSize, uint32_t maxFiles,
                                         bool rotateOnOpen)
{
    if (file == nullptr) {
        return new struct SinkSt(std::make_shared<RotatingFileSink>());
    } else {
        return new struct SinkSt(
            std::make_shared<DailyFileSink>(file, maxFileSize, maxFiles, rotateOnOpen));
    }
}

void origin_detroy_sink(SinkSt *sink)
{
    if (sink != nullptr) {
        if (sink->ptr != nullptr) {
            ORIGIN_DEBUG_DBG(
                "Release SinkSt. UseCnt: {}. {}", sink->ptr.use_count(), sink->ptr->param_str());
            sink->ptr.reset();
        }
        delete sink;
    }
}

void origin_sink_set_level(const SinkSt *sink, OriginLogLevel level)
{
    RETURN_IF_PTR_NULL(sink);
    sink->ptr->set_level(c_to_cpp_log_level(level));
}

bool origin_sink_should_log(const SinkSt *sink, OriginLogLevel level)
{
    RETURN_VALUE_IF_PTR_NULL(sink, false);
    return sink->ptr->should_log(c_to_cpp_log_level(level));
}

OriginLogLevel origin_sink_level(const SinkSt *sink)
{
    RETURN_VALUE_IF_PTR_NULL(sink, ORIGIN_LOG_LEVEL_OFF);
    return cpp_to_c_log_level(sink->ptr->level());
}

void origin_sink_set_pattern(const SinkSt *sink, const char *pattern)
{
    RETURN_IF_PTR_NULL(sink);
    RETURN_IF_PTR_NULL(pattern);
    sink->ptr->set_pattern(pattern);
}

void origin_sink_set_formatter(const SinkSt *sink, const FormatterSt *formatter)
{
    RETURN_IF_PTR_NULL(sink);
    RETURN_IF_PTR_NULL(formatter);
    RETURN_IF_PTR_NULL(formatter->ptr);
    sink->ptr->set_formatter(formatter->ptr->clone());
}
}
