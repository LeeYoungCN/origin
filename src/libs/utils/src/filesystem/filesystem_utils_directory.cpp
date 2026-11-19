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

#include <exception>
#include <filesystem>
#include <string_view>

#include "filesystem/internal/common.hpp"
#include "origin/common/common_error_code.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/common/types/filesystem_types.h"
#include "origin/filesystem/filesystem.h"
#include "origin/thread/thread.h"
#include "origin/utils/utils_error_code.h"

namespace origin::filesystem {

namespace fs = std::filesystem;
using namespace origin::thread;

inline const char* recursive_mode_str(bool recursive)
{
    return (recursive ? "recursive" : "not recursive");
}

bool dir_exists(std::string_view path)
{
    EntryType type = EntryType::UNKNOWN;
    (void)get_entry_type(path, type);
    return type == EntryType::DIRECTORY;
}

ErrorCode create_dir(std::string_view path, bool recursive)
{
    EntryType type = EntryType::UNKNOWN;
    (void)get_entry_type(path, type);
    if (type == EntryType::DIRECTORY) {
        ORIGIN_DEBUG_TRACE("Create dir success. dir: \"{}\", mode: {}. message: \"already exist\".",
                           path.data(),
                           recursive_mode_str(recursive));
        return ERR_COMM_SUCCESS;
    }
    if (type != EntryType::NONEXISTENT) {
        ORIGIN_DEBUG_ERR("Create dir failed. dir: \"{}\", mode: {}. message: \"Target type {}\".",
                         path.data(),
                         recursive_mode_str(recursive),
                         get_entry_type_str(type));
        return ERR_UTILS_NOT_DIRECTORY;
    }

    try {
        if (recursive) {
            fs::create_directories(path);
        } else {
            fs::create_directory(path);
        }
        ORIGIN_DEBUG_TRACE(
            "Create dir {} success. dir: \"{}\".", recursive_mode_str(recursive), path.data());
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR("Create dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive_mode_str(recursive),
                         path.data(),
                         get_utils_err_msg(code));
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR("Create dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive_mode_str(recursive),
                         path.data(),
                         get_utils_err_msg(code));
        return code;
    }
}

ErrorCode delete_dir(std::string_view path, bool recursive)
{
    EntryType type = EntryType::UNKNOWN;
    (void)get_entry_type(path, type);
    if (type == EntryType::NONEXISTENT) {
        return ERR_COMM_SUCCESS;
    }
    if (type != EntryType::DIRECTORY) {
        return ERR_UTILS_NOT_DIRECTORY;
    }

    try {
        bool result = false;
        if (recursive) {
            result = fs::remove_all(path) > 0;
        } else {
            result = fs::remove(path);  // 非递归删除，目录必须为空
        }
        ORIGIN_DEBUG_TRACE("Delete dir {} success. dir: \"{}\". result: {}.",
                           recursive ? "recursive" : "not recursive",
                           path.data(),
                           result);
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR("Delete dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive ? "recursive" : "not recursive",
                         path.data(),
                         get_utils_err_msg(code));
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR("Delete dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive ? "recursive" : "not recursive",
                         path.data(),
                         get_utils_err_msg(code));
        return code;
    }
}

}  // namespace origin::filesystem
