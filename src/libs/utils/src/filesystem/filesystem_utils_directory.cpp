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
#include "../internal/last_error.h"
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
    bool result = false;
    switch (type) {
        case EntryType::DIRECTORY:
            result = true;
            set_last_error(ERR_COMM_SUCCESS);
            break;
        case EntryType::NONEXISTENT:
            set_last_error(ERR_UTILS_NOT_FOUND);
            break;
        default:
            set_last_error(ERR_UTILS_NOT_DIRECTORY);
            break;
    }
    return result;
}

ErrorCode create_dir(std::string_view path, bool recursive)
{
    EntryType type = EntryType::UNKNOWN;
    (void)get_entry_type(path, type);
    if (type == EntryType::DIRECTORY) {
        set_last_error(ERR_UTILS_ALREADY_EXISTS);
        ORIGIN_DEBUG_TRACE("Create dir success. dir: \"{}\", mode: {}. message: \"already exist\".",
                           path.data(),
                           recursive_mode_str(recursive));
        return ERR_COMM_SUCCESS;
    }
    if (type != EntryType::NONEXISTENT) {
        set_last_error(ERR_UTILS_NOT_DIRECTORY);
        ORIGIN_DEBUG_ERR("Create dir failed. dir: \"{}\", mode: {}. message: \"Target type {}\".",
                         path.data(),
                         recursive_mode_str(recursive),
                         get_entry_type_str(type));
        return get_last_error();
    }

    try {
        if (recursive) {
            fs::create_directories(path);
        } else {
            fs::create_directory(path);
        }
        ORIGIN_DEBUG_TRACE(
            "Create dir {} success. dir: \"{}\".", recursive_mode_str(recursive), path.data());
        set_last_error(ERR_COMM_SUCCESS);
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        set_last_error(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Create dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive_mode_str(recursive),
                         path.data(),
                         get_last_error_msg());
        return get_last_error();
    } catch (const std::exception& e) {
        set_last_error(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Create dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive_mode_str(recursive),
                         path.data(),
                         get_last_error_msg());
        return get_last_error();
    }
}

ErrorCode delete_dir(std::string_view path, bool recursive)
{
    if (!dir_exists(path)) {
        bool const rst = (get_last_error() == ERR_UTILS_NOT_FOUND);
        if (!rst) {
            ORIGIN_DEBUG_ERR("Delete dir {} failed. dir: \"{}\". msg: \"{}\".",
                             recursive ? "recursive" : "not recursive",
                             path.data(),
                             get_last_error_msg());
        } else {
            ORIGIN_DEBUG_TRACE("Delete dir {} success. dir: \"{}\". msg: \"{}\".",
                               recursive ? "recursive" : "not recursive",
                               path.data(),
                               get_last_error_msg());
        }
        return rst ? ERR_COMM_SUCCESS : get_last_error();
    }
    try {
        bool result = false;
        if (recursive) {
            result = fs::remove_all(path) > 0;
        } else {
            result = fs::remove(path);  // 非递归删除，目录必须为空
        }
        set_last_error(result ? ERR_COMM_SUCCESS : ERR_UTILS_NOT_FOUND);
        ORIGIN_DEBUG_TRACE("Delete dir {} success. dir: \"{}\". msg: \"{}\".",
                           recursive ? "recursive" : "not recursive",
                           path.data(),
                           get_last_error_msg());
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        set_last_error(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Delete dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive ? "recursive" : "not recursive",
                         path.data(),
                         get_last_error_msg());
        return get_last_error();
    } catch (const std::exception& e) {
        set_last_error(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Delete dir {} failed. dir: \"{}\". msg: \"{}\".",
                         recursive ? "recursive" : "not recursive",
                         path.data(),
                         get_last_error_msg());
        return get_last_error();
    }
}

}  // namespace origin::filesystem
