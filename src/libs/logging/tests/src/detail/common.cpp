#include "common.h"

#include <format>

#include "utils/filesystem_utils.h"
#include "utils/process_utils.h"

namespace logging_test {
using namespace origin;
std::string get_logger_name(const testing::TestInfo* test_info)
{
    return std::format("{}.{}", test_info->test_suite_name(), test_info->name());
}

std::string get_log_dir()
{
    return filesystem::join_paths({process::get_proc_directory(), "logs"});
}
}  // namespace logging_test
