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
#include "internal/last_error.h"

#if OS_WINDOWS
#include <windows.h>

#include <libloaderapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>

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
#include "origin/common/types/process_types.h"
#include "origin/filesystem/filesystem.h"
#include "origin/thread/thread.h"

namespace origin::process {
using namespace origin::filesystem;
using namespace origin::thread;

ErrorCode get_proc_path(std::string& out)
{
    char path[MAX_PATH_STD] = {'\0'};
#if OS_WINDOWS
    DWORD length = GetModuleFileNameA(nullptr, path, MAX_PATH_STD);
    if (length == 0 || length >= MAX_PATH_STD) {
        set_last_error(ERR_COMM_SYSTEM_ERROR);
        ORIGIN_DEBUG_ERR("[WIN32] Get process path failed, length: {}", length);
        return get_last_error();
    }
#elif OS_LINUX
    auto length = readlink("/proc/self/exe", path, MAX_PATH_STD - 1);
    if (length == -1) {
        set_last_error(ERR_COMM_SYSTEM_ERROR);
        ORIGIN_DEBUG_ERR("[Linux] Get process path failed.");
        return get_last_error();
    }
    path[length] = '\0';
#elif OS_MACOS
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        set_last_error(ERR_COMM_SYSTEM_ERROR);
        ORIGIN_DEBUG_ERR("[Macos] Get process path failed.");
        return get_last_error();
    }
#else
#error "Unsupport system."
#endif
    out = path;
    set_last_error(ERR_COMM_SUCCESS);
    ORIGIN_DEBUG_TRACE("Get process path: {}", path);
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
    set_last_error(ERR_COMM_SUCCESS);
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
    set_last_error(ERR_COMM_SUCCESS);
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
