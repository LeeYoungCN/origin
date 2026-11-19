#include "internal/common.hpp"

#include <cstdint>
#include <string>
#include <string_view>

#include "origin/common/common_error_code.h"
#include "origin/utils/utils_error_code.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/date_time/date_time.h"
#include "origin/filesystem/filesystem.h"
#include "origin/process/process.h"

namespace origin::logging {
using namespace origin::filesystem;
using namespace origin::process;
using namespace origin::date_time;

std::string get_default_log_file(const std::string_view suffix)
{
    const std::string process = get_proc_path();
    return join_paths(
        {get_directory(process), "logs", get_filename_stem(process) + "." + std::string(suffix)});
}

bool delete_file_retry(const std::string_view file, const uint32_t maxRetry, const uint32_t sleepMs)
{
    ErrorCode lastError = ERR_COMM_SUCCESS;
    for (uint32_t i = 0; i < maxRetry; i++) {
        lastError = delete_file(file);
        if (lastError == ERR_COMM_SUCCESS) {
            ORIGIN_DEBUG_TRACE("Delete file success. file: \"{}\".", file);
            return true;
        }
        sleep_ms(sleepMs);
    }

    ORIGIN_DEBUG_ERR("Delete file failed. file: \"{}\", msg: \"{}\".", file,
                     get_utils_err_msg(lastError));
    return false;
}

bool rename_file_retry(const std::string_view src, std::string_view dest, const bool overwrite,
                       const uint32_t maxRetry, const uint32_t sleepMs)
{
    ErrorCode lastError = ERR_COMM_SUCCESS;
    for (uint32_t i = 0; i < maxRetry; i++) {
        lastError = rename_file(src, dest, overwrite);
        if (lastError == ERR_COMM_SUCCESS) {
            ORIGIN_DEBUG_TRACE("Rename file success. file: \"{}\".", dest);
            return true;
        }
        sleep_ms(sleepMs);
    }

    ORIGIN_DEBUG_ERR("Rename file failed. file: \"{}\", msg: \"{}\".", dest,
                     get_utils_err_msg(lastError));
    return false;
}
}  // namespace origin::logging
