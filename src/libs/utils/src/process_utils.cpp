/**
 * @file process_utils.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2026-02-05
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "origin/process/process.h"

#if OS_WINDOWS
#include <cstddef>

// SDK 窄头文件依赖 windows.h 先建立目标架构与基础类型；include-cleaner
// 无法识别这种依赖，因此保留 windows.h 并显式标记为需要。
// NOLINTNEXTLINE(misc-include-cleaner)
#include <windows.h>  // IWYU pragma: keep

#include <errhandlingapi.h>
#include <libloaderapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <stringapiset.h>
#include <winnls.h>

#include <utility>

#include "origin/utils/utils_error_code.h"

#elif OS_LINUX
#include <unistd.h>  // Linux的readlink函数
#elif OS_MACOS
#include <mach-o/dyld.h>  // macOS的_NSGetExecutablePath
#include <unistd.h>

#include <cstdint>
#endif
#include <string>

#include "origin/common/common_error_code.h"
#include "origin/common/constants/filesystem_constants.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/common/macros/compiler.h"
#include "origin/common/types/error_code_types.h"
#include "origin/common/types/process_types.h"

#include "origin/filesystem/filesystem.h"

namespace origin::process {
using namespace origin::filesystem;

ErrorCode get_proc_path(std::string& out)
{
#if OS_WINDOWS
    // 使用宽字符 API 并按返回长度扩容，避免 ANSI 编码和 MAX_PATH 截断。
    std::wstring widePath(MAX_PATH_STD, L'\0');
    DWORD length = 0;
    while (true) {
        length = GetModuleFileNameW(nullptr, widePath.data(), static_cast<DWORD>(widePath.size()));
        if (length == 0) {
            ORIGIN_DEBUG_ERR("[WIN32] Get process path failed, error: {}", GetLastError());
            return ERR_COMM_SYSTEM_ERROR;
        }
        if (length < widePath.size()) {
            break;
        }
        if (widePath.size() >= MAX_PATH_LONG) {
            ORIGIN_DEBUG_ERR("[WIN32] Get process path is too long.");
            return ERR_UTILS_PATH_TOO_LONG;
        }
        widePath.resize(widePath.size() * 2);
    }
    widePath.resize(length);

    const int utf8Length = WideCharToMultiByte(CP_UTF8,
                                               WC_ERR_INVALID_CHARS,
                                               widePath.data(),
                                               static_cast<int>(widePath.size()),
                                               nullptr,
                                               0,
                                               nullptr,
                                               nullptr);
    if (utf8Length <= 0) {
        ORIGIN_DEBUG_ERR("[WIN32] Convert process path to UTF-8 failed.");
        return ERR_UTILS_PATH_INVALID;
    }
    std::string utf8Path(static_cast<size_t>(utf8Length), '\0');
    if (WideCharToMultiByte(CP_UTF8,
                            WC_ERR_INVALID_CHARS,
                            widePath.data(),
                            static_cast<int>(widePath.size()),
                            utf8Path.data(),
                            utf8Length,
                            nullptr,
                            nullptr) <= 0) {
        ORIGIN_DEBUG_ERR("[WIN32] Convert process path to UTF-8 failed.");
        return ERR_UTILS_PATH_INVALID;
    }
    out = std::move(utf8Path);
    ORIGIN_DEBUG_TRACE("Get process path: {}", out);
#elif OS_LINUX
    char path[MAX_PATH_STD] = {'\0'};
    auto length = readlink("/proc/self/exe", path, MAX_PATH_STD - 1);
    if (length == -1) {
        ORIGIN_DEBUG_ERR("[Linux] Get process path failed.");
        return ERR_COMM_SYSTEM_ERROR;
    }
    path[length] = '\0';
    out = path;
    ORIGIN_DEBUG_TRACE("Get process path: {}", path);
#elif OS_MACOS
    char path[MAX_PATH_STD] = {'\0'};
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        ORIGIN_DEBUG_ERR("[Macos] Get process path failed.");
        return ERR_COMM_SYSTEM_ERROR;
    }
    out = path;
    ORIGIN_DEBUG_TRACE("Get process path: {}", path);
#else
#error "Unsupport system."
#endif
    return ERR_COMM_SUCCESS;
}

ErrorCode get_proc_directory(std::string& out)
{
    std::string path;
    const ErrorCode rc = get_proc_path(path);
    if (rc != ERR_COMM_SUCCESS) {
        return rc;
    }
    out = get_directory(path);
    return ERR_COMM_SUCCESS;
}

ErrorCode get_proc_filename_stem(std::string& out)
{
    std::string path;
    const ErrorCode rc = get_proc_path(path);
    if (rc != ERR_COMM_SUCCESS) {
        return rc;
    }
    out = get_filename_stem(path);
    return ERR_COMM_SUCCESS;
}

ProcessId get_curr_proc_id()
{
    ProcessId pid = 0;
#if OS_WINDOWS
    pid = GetCurrentProcessId();
#else
    pid = getpid();
#endif
    return pid;
}
}  // namespace origin::process
