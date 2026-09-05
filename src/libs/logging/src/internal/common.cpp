#include "internal/common.h"

#include "common/debug/debug_logger.h"
#include "utils/date_time_utils.h"
#include "utils/filesystem_utils.h"
#include "utils/process_utils.h"
#include "utils/thread_utils.h"

namespace origin::logging {
using namespace origin::filesystem;
using namespace origin::process;

std::string get_default_log_file(std::string_view suffix)
{
    std::string process = get_proc_path();
    return join_paths(
        {get_directory(process), "logs", get_filename_stem(process) + "." + std::string(suffix)});
}

bool delete_file(std::string_view file, uint32_t maxRetry, uint32_t sleepMs)
{
    for (uint32_t i = 0; i < maxRetry; i++) {
        if (filesystem::delete_file(file)) {
            ORIGIN_DEBUG_TRACE("Delete file success. file: \"{}\".", file);
            return true;
        }
        origin::date_time::sleep_ms(sleepMs);
    }

    ORIGIN_DEBUG_ERR(
        "Delete file failed. file: \"{}\", msg: \"{}\".", file, get_thread_last_err_msg());
    return false;
}

bool rename_file(std::string_view src, std::string_view dest, bool overwrite, uint32_t maxRetry,
                 uint32_t sleepMs)
{
    for (uint32_t i = 0; i < maxRetry; i++) {
        if (filesystem::rename_file(src, dest, overwrite)) {
            ORIGIN_DEBUG_TRACE("Rename file success. file: \"{}\".", dest);
            return true;
        }
        origin::date_time::sleep_ms(sleepMs);
    }

    ORIGIN_DEBUG_ERR(
        "Rename file failed. file: \"{}\", msg: \"{}\".", dest, get_thread_last_err_msg());
    return false;
}

}  // namespace origin::logging
