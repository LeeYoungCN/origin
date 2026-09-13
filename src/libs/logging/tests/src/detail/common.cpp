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
        ORIGIN_DEBUG_DBG("Release SinkSt. UseCnt: {}. {}",
                         mockSink->ptr.use_count(),
                         mockSink->ptr->param_str());
        mockSink->ptr.reset();
    }
    delete mockSink;
}
}  // namespace logging_test
