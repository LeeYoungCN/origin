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

#include <cstddef>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>

#include "common/constants/filesystem_constants.h"
#include "common/debug/debug_logger.h"
#include "internal/utils/filesystem_utils_internal.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

namespace origin::filesystem {
namespace fs = std::filesystem;
using namespace origin::filesystem::internal;
using namespace origin::filesystem;

// ------------------------------ 系统路径接口 ------------------------------

std::string get_curr_working_dir()
{
    try {
        auto p = fs::current_path();
        set_thread_last_err(ERR_COMM_SUCCESS);
        return p.string();
    } catch (const fs::filesystem_error& e) {
        ORIGIN_DEBUG_ERR("Get current working dir failed.");
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        return "";
    } catch (const std::exception& e) {
        ORIGIN_DEBUG_ERR("Get current working dir failed.");
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        return "";
    }
}

// ------------------------------ 路径处理接口 ------------------------------
std::string join_paths(const std::vector<std::string>& parts)
{
    if (parts.empty()) {
        set_thread_last_err(ERR_UTILS_PATH_INVALID);
        ORIGIN_DEBUG_WARN("Join paths failed. list empty: {}", get_thread_last_err_msg());
        return "";
    }
    fs::path result(parts[0]);
    for (size_t i = 1; i < parts.size(); ++i) {
        result /= parts[i];  // std::filesystem 自动处理分隔符
    }
    set_thread_last_err(ERR_COMM_SUCCESS);
    return result.string();
}

std::string normalize_path(std::string_view path)
{
    try {
        auto normalized = fs::path(path).lexically_normal();
        set_thread_last_err(ERR_COMM_SUCCESS);
        return normalized.string();
    } catch (const fs::filesystem_error& e) {
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Normalized path faild. path: \"{}\", message: {}. ex: {}",
                         path,
                         get_thread_last_err_msg(),
                         e.what());
        return "";
    } catch (const std::exception& e) {
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Normalized path faild. path: \"{}\", message: {}. ex: {}",
                         path,
                         get_thread_last_err_msg(),
                         e.what());
        return "";
    }
}

std::string to_absolute_path(std::string_view relPath, std::string_view baseDir)
{
    if (relPath.empty()) {
        return "";
    }
    fs::path base = fs::path(baseDir.empty() ? get_curr_working_dir() : baseDir);
    fs::path relative(relPath);
    fs::path combined = base / relative;

    try {
        auto absPath = fs::absolute(combined).lexically_normal();
        set_thread_last_err(ERR_COMM_SUCCESS);
        return absPath.string();
    } catch (const fs::filesystem_error& e) {
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Absolute path faild. path: \"{}\", message: {}. ex: {}",
                         relPath,
                         get_thread_last_err_msg(),
                         e.what());
        return "";
    } catch (const std::exception& e) {
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Absolute path faild. path: \"{}\", message: {}.ex: {}",
                         relPath,
                         get_thread_last_err_msg(),
                         e.what());
        return "";
    }
}

std::string get_directory(std::string_view path)
{
    fs::path proc(path);
    set_thread_last_err(ERR_COMM_SUCCESS);
    return proc.parent_path().string();
}

std::string get_filename(std::string_view path)
{
    fs::path p(path);
    set_thread_last_err(ERR_COMM_SUCCESS);
    return p.filename().string();
}

std::string get_filename_stem(std::string_view path)
{
    fs::path p(path);
    set_thread_last_err(ERR_COMM_SUCCESS);
    return p.stem().string();
}

std::string get_extension(std::string_view path)
{
    fs::path p(path);
    set_thread_last_err(ERR_COMM_SUCCESS);
    return p.extension().string();
}

bool is_absolute_path(std::string_view path)
{
    bool result = fs::path(path).is_absolute();
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
