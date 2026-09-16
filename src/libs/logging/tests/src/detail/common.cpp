#include "detail/common.hpp"

#include <format>
#include <string>
#include <utility>

#include "common/debug/debug_logger.h"
#include "utils/filesystem_utils.h"
#include "utils/process_utils.h"

namespace logging_test {
using namespace origin;
using namespace origin::logging;

int32_t diff_log_level(LogLevel a, LogLevel b)
{
    return static_cast<int32_t>(a) - static_cast<int32_t>(b);
}

std::string get_logger_name(const testing::TestInfo* test_info)
{
    return std::format("{}.{}", test_info->test_suite_name(), test_info->name());
}

std::string get_log_dir()
{
    return filesystem::join_paths(
        {process::get_proc_directory(), "logs_" + std::to_string(process::get_curr_proc_id())});
}

SinkSt* create_mock_sink_st(std::shared_ptr<origin::logging::Sink> sink)
{
    auto mockSink = new MockSinkSt(std::move(sink));
    return reinterpret_cast<SinkSt*>(mockSink);
}

void destroy_mock_sink_st(SinkSt* sink)
{
    if (sink == nullptr) {
        return;
    }

    auto mockSink = reinterpret_cast<MockSinkSt*>(sink);

    if (mockSink->ptr != nullptr) {
        ORIGIN_DEBUG_DBG("Release MockSinkSt. UseCnt: {}. {}.",
                         mockSink->ptr.use_count(),
                         mockSink->ptr->param_str());
        mockSink->ptr.reset();
    }
    delete mockSink;
}

LoggerSt* create_mock_logger_st(std::shared_ptr<origin::logging::Logger> logger)
{
    auto mockLogger = new MockLoggerSt(std::move(logger));
    return reinterpret_cast<LoggerSt*>(mockLogger);
}

void destroy_mock_logger_st(LoggerSt* logger)
{
    if (logger == nullptr) {
        return;
    }

    const auto mockLogger = reinterpret_cast<MockLoggerSt*>(logger);

    if (mockLogger->ptr != nullptr) {
        ORIGIN_DEBUG_DBG("Release MockLoggerSt. UseCnt: {}. Name: {}.",
                         mockLogger->ptr.use_count(),
                         mockLogger->ptr->name());
        mockLogger->ptr.reset();
    }
    delete mockLogger;
}

FormatterSt* create_mock_formatter_st(std::unique_ptr<origin::logging::Formatter> formatter)
{
    const auto mockLogger = new MockFormatterSt(std::move(formatter));
    return reinterpret_cast<FormatterSt*>(mockLogger);
}
void destroy_mock_formatter_st(FormatterSt* formatter)
{
    if (formatter == nullptr) {
        return;
    }

    const auto mockFormatter = reinterpret_cast<MockFormatterSt*>(formatter);

    if (mockFormatter->ptr != nullptr) {
        ORIGIN_DEBUG_DBG("Release MockFormatterSt.");
        mockFormatter->ptr.reset();
    }
    delete mockFormatter;
}

}  // namespace logging_test
