/**
 * @file filesystem_utils.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-30
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "common/compiler/macros.h"
#include "utils/filesystem_utils.h"

#if OS_WINDOWS
#include <windows.h>
#elif OS_LINUX
#include <unistd.h>  // Linux的readlink函数
#elif OS_MACOS
#include <mach-o/dyld.h>  // macOS的_NSGetExecutablePath
#endif

#include <exception>
#include <filesystem>

#include "common/debug/debug_logger.h"
#include "common/types/filesystem_types.h"
#include "internal/utils/filesystem_utils_internal.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

namespace origin::filesystem {

namespace fs = std::filesystem;
using namespace origin::filesystem::internal;

inline const char* recursive_mode_str(bool recursive)
{
    return (recursive ? "recursive" : "not recursive");
}

bool dir_exists(std::string_view path)
{
    EntryType type = get_entry_type(path);
    bool result = false;
    switch (type) {
        case EntryType::DIRECTORY:
            result = true;
            set_thread_last_err(ERR_COMM_SUCCESS);
            break;
        case EntryType::NONEXISTENT:
            set_thread_last_err(ERR_UTILS_NOT_FOUND);
            break;
        default:
            set_thread_last_err(ERR_UTILS_NOT_DIRECTORY);
            break;
    }
    return result;
}

bool create_dir(std::string_view path, bool recursive)
{
    auto type = get_entry_type(path);
    if (type == EntryType::DIRECTORY) {
        set_thread_last_err(ERR_UTILS_ALREADY_EXISTS);
        ORIGIN_DEBUG_TRACE("Create dir success. dir: \"{}\", mode: {}. message: \"already exist\".",
                           path.data(),
                           recursive_mode_str(recursive));
        return true;
    }
    if (type != EntryType::NONEXISTENT) {
        set_thread_last_err(ERR_UTILS_NOT_DIRECTORY);
        ORIGIN_DEBUG_ERR("Create dir failed. dir: \"{}\", mode: {}. message: \"Target type {}\".",
                         path.data(),
                         recursive_mode_str(recursive),
                         get_entry_type_str(type));
        return false;
    }

    try {
        if (recursive) {
            fs::create_directories(path);
        } else {
            fs::create_directory(path);
        }
        ORIGIN_DEBUG_TRACE(
            "Create dir {} success. dir: \"{}\".", recursive_mode_str(recursive), path.data());
        set_thread_last_err(ERR_COMM_SUCCESS);
        return true;
    } catch (const fs::filesystem_error& e) {
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Create dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive_mode_str(recursive),
                         path.data(),
                         get_thread_last_err_msg());
        return false;
    } catch (const std::exception& e) {
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Create dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive_mode_str(recursive),
                         path.data(),
                         get_thread_last_err_msg());
        return false;
    }
}

bool delete_dir(std::string_view path, bool recursive)
{
    if (!dir_exists(path)) {
        bool rst = (get_thread_last_err() == ERR_UTILS_NOT_FOUND);
        if (!rst) {
            ORIGIN_DEBUG_ERR("Delete dir {} failed. dir: \"{}\". msg: \"{}\".",
                             recursive ? "recursive" : "not recursive",
                             path.data(),
                             get_thread_last_err_msg());
        } else {
            ORIGIN_DEBUG_TRACE("Delete dir {} success. dir: \"{}\". msg: \"{}\".",
                               recursive ? "recursive" : "not recursive",
                               path.data(),
                               get_thread_last_err_msg());
        }
        return rst;
    }
    try {
        bool result = false;
        if (recursive) {
            result = fs::remove_all(path) > 0;
        } else {
            result = fs::remove(path);  // 非递归删除，目录必须为空
        }
        set_thread_last_err(result ? ERR_COMM_SUCCESS : ERR_UTILS_NOT_FOUND);
        ORIGIN_DEBUG_TRACE("Delete dir {} success. dir: \"{}\". msg: \"{}\".",
                           recursive ? "recursive" : "not recursive",
                           path.data(),
                           get_thread_last_err_msg());
        return true;
    } catch (const fs::filesystem_error& e) {
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Delete dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive ? "recursive" : "not recursive",
                         path.data(),
                         get_thread_last_err_msg());
        return false;
    } catch (const std::exception& e) {
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Delete dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive ? "recursive" : "not recursive",
                         path.data(),
                         get_thread_last_err_msg());
        return false;
    }
}

}  // namespace origin::filesystem
