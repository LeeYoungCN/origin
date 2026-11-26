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

#include "origin/common/common_error_code.h"
#include "origin/common/constants/filesystem_constants.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/common/macros/compiler.h"

#if OS_WINDOWS
// SDK 的窄头文件依赖 windows.h 先建立目标架构与基础类型；include-cleaner
// 无法识别这种依赖，因此保留 windows.h 并显式标记为需要。
// NOLINTNEXTLINE(misc-include-cleaner)
#include <windows.h>  // IWYU pragma: keep

#include <stringapiset.h>
#include <winnls.h>
#endif

#include "origin/common/types/error_code_types.h"

#include "origin/filesystem/filesystem.h"
#include "origin/utils/utils_error_code.h"

#include "filesystem/internal/common.hpp"

namespace origin::filesystem {
namespace fs = std::filesystem;

// ------------------------------ 系统路径接口 ------------------------------

ErrorCode get_curr_working_dir(std::string& out)
{
    try {
        out = fs::current_path().string();
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR("Get current working dir failed: {}", get_utils_err_msg(code));
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR("Get current working dir failed: {}", get_utils_err_msg(code));
        return code;
    }
}

// ------------------------------ 路径处理接口 ------------------------------
ErrorCode join_paths(const std::vector<std::string>& parts, std::string& out)
{
    if (parts.empty()) {
        ORIGIN_DEBUG_WARN("Join paths failed: empty list.");
        return ERR_UTILS_PATH_INVALID;
    }
    try {
        fs::path result(parts[0]);
        for (size_t i = 1; i < parts.size(); ++i) {
            result /= parts[i];  // std::filesystem 自动处理分隔符
        }
        out = result.string();
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR(
            "Join paths failed. message: {}. ex: {}", get_utils_err_msg(code), e.what());
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR(
            "Join paths failed. message: {}. ex: {}", get_utils_err_msg(code), e.what());
        return code;
    }
}

ErrorCode normalize_path(std::string_view path, std::string& out)
{
    try {
        out = fs::path(path).lexically_normal().string();
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR("Normalize path failed. path: \"{}\", message: {}. ex: {}",
                         path,
                         get_utils_err_msg(code),
                         e.what());
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR("Normalize path failed. path: \"{}\", message: {}. ex: {}",
                         path,
                         get_utils_err_msg(code),
                         e.what());
        return code;
    }
}

ErrorCode to_absolute_path(std::string_view relPath, std::string_view baseDir, std::string& out)
{
    if (relPath.empty()) {
        out.clear();
        return ERR_COMM_SUCCESS;
    }

    try {
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
        out = fs::absolute(combined).lexically_normal().string();
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR("Absolute path failed. path: \"{}\", message: {}. ex: {}",
                         relPath,
                         get_utils_err_msg(code),
                         e.what());
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR("Absolute path failed. path: \"{}\", message: {}. ex: {}",
                         relPath,
                         get_utils_err_msg(code),
                         e.what());
        return code;
    }
}

std::string get_directory(std::string_view path)
{
    try {
        fs::path const proc(path);
        return proc.parent_path().string();
    } catch (...) {
        return {};
    }
}

std::string get_filename(std::string_view path)
{
    try {
        fs::path const p(path);
        return p.filename().string();
    } catch (...) {
        return {};
    }
}

std::string get_filename_stem(std::string_view path)
{
    try {
        fs::path const p(path);
        return p.stem().string();
    } catch (...) {
        return {};
    }
}

std::string get_extension(std::string_view path)
{
    try {
        fs::path const p(path);
        return p.extension().string();
    } catch (...) {
        return {};
    }
}

bool is_absolute_path(std::string_view path)
{
    try {
        return fs::path(path).is_absolute();
    } catch (...) {
        return false;
    }
}

bool is_path_too_long(std::string_view path)
{
#if OS_WINDOWS
    // Windows 路径长度按 UTF-16 码元计算，不能直接用 UTF-8 字节数。
    const int wideLength = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, path.data(), static_cast<int>(path.size()), nullptr, 0);
    if (wideLength <= 0) {
        // 无法转换的路径不可能被 Windows API 使用，按“过长”处理。
        return true;
    }
    return static_cast<size_t>(wideLength) > MAX_PATH_LONG;
#else
    return path.length() > MAX_PATH_STD;
#endif
}

}  // namespace origin::filesystem
