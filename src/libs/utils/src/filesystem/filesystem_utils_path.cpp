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
    const auto len = path.length();
    bool result = false;
#if OS_WINDOWS
    result = len > MAX_PATH_LONG;
#else
    result = len > MAX_PATH_STD;
#endif
    return result;
}

}  // namespace origin::filesystem
