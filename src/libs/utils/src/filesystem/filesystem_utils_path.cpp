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

#include <cstddef>
#include <exception>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "filesystem/internal/common.hpp"
#include "origin/common/common_error_code.h"
#include "origin/common/constants/filesystem_constants.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/common/macros/compiler.h"
#include "origin/filesystem/filesystem.h"
#include "origin/thread/thread.h"
#include "origin/utils/utils_error_code.h"

namespace origin::filesystem {
namespace fs = std::filesystem;
using namespace origin::thread;

// ------------------------------ 系统路径接口 ------------------------------

ErrorCode get_curr_working_dir(std::string& out)
{
    try {
        out = fs::current_path().string();
        set_thread_last_err(ERR_COMM_SUCCESS);
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        ORIGIN_DEBUG_ERR("Get current working dir failed.");
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        return get_thread_last_err();
    } catch (const std::exception& e) {
        ORIGIN_DEBUG_ERR("Get current working dir failed.");
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        return get_thread_last_err();
    }
}

// ------------------------------ 路径处理接口 ------------------------------
ErrorCode join_paths(const std::vector<std::string>& parts, std::string& out)
{
    if (parts.empty()) {
        set_thread_last_err(ERR_UTILS_PATH_INVALID);
        ORIGIN_DEBUG_WARN("Join paths failed. list empty: {}", get_thread_last_err_msg());
        return ERR_UTILS_PATH_INVALID;
    }
    fs::path result(parts[0]);
    for (size_t i = 1; i < parts.size(); ++i) {
        result /= parts[i];  // std::filesystem 自动处理分隔符
    }
    out = result.string();
    set_thread_last_err(ERR_COMM_SUCCESS);
    return ERR_COMM_SUCCESS;
}

ErrorCode normalize_path(std::string_view path, std::string& out)
{
    try {
        out = fs::path(path).lexically_normal().string();
        set_thread_last_err(ERR_COMM_SUCCESS);
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Normalized path failed. path: \"{}\", message: {}. ex: {}",
                         path,
                         get_thread_last_err_msg(),
                         e.what());
        return get_thread_last_err();
    } catch (const std::exception& e) {
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Normalized path failed. path: \"{}\", message: {}. ex: {}",
                         path,
                         get_thread_last_err_msg(),
                         e.what());
        return get_thread_last_err();
    }
}

ErrorCode to_absolute_path(std::string_view relPath, std::string_view baseDir, std::string& out)
{
    if (relPath.empty()) {
        out.clear();
        return ERR_COMM_SUCCESS;
    }

    std::string base;
    if (baseDir.empty()) {
        const ErrorCode rc = get_curr_working_dir(base);
        if (rc != ERR_COMM_SUCCESS) {
            return rc;
        }
    } else {
        base.assign(baseDir);
    }

    fs::path const combined = fs::path(base) / fs::path(relPath);
    try {
        out = fs::absolute(combined).lexically_normal().string();
        set_thread_last_err(ERR_COMM_SUCCESS);
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Absolute path failed. path: \"{}\", message: {}. ex: {}",
                         relPath,
                         get_thread_last_err_msg(),
                         e.what());
        return get_thread_last_err();
    } catch (const std::exception& e) {
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Absolute path failed. path: \"{}\", message: {}.ex: {}",
                         relPath,
                         get_thread_last_err_msg(),
                         e.what());
        return get_thread_last_err();
    }
}

std::string get_directory(std::string_view path)
{
    fs::path const proc(path);
    set_thread_last_err(ERR_COMM_SUCCESS);
    return proc.parent_path().string();
}

std::string get_filename(std::string_view path)
{
    fs::path const p(path);
    set_thread_last_err(ERR_COMM_SUCCESS);
    return p.filename().string();
}

std::string get_filename_stem(std::string_view path)
{
    fs::path const p(path);
    set_thread_last_err(ERR_COMM_SUCCESS);
    return p.stem().string();
}

std::string get_extension(std::string_view path)
{
    fs::path const p(path);
    set_thread_last_err(ERR_COMM_SUCCESS);
    return p.extension().string();
}

bool is_absolute_path(std::string_view path)
{
    bool const result = fs::path(path).is_absolute();
    set_thread_last_err(ERR_COMM_SUCCESS);
    return result;
}

bool is_path_too_long(std::string_view path)
{
    const auto len = path.length();
    bool result = false;
#if OS_WINDOWS
    result = len > MAX_PATH_LONG;
#else
    result = len > MAX_PATH_STD;
#endif
    set_thread_last_err(ERR_COMM_SUCCESS);
    return result;
}

}  // namespace origin::filesystem
