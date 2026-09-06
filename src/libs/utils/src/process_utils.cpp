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
#include "utils/process_utils.h"

#if OS_WINDOWS
#include <windows.h>
#elif OS_LINUX
#include <unistd.h>  // Linux的readlink函数
#elif OS_MACOS
#include <mach-o/dyld.h>  // macOS的_NSGetExecutablePath
#include <unistd.h>
#endif

#include "common/common_error_code.h"
#include "common/constants/filesystem_constants.h"
#include "common/debug/debug_logger.h"
#include "common/macros/compiler.h"
#include "common/types/process_types.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"

namespace origin::process {
using namespace origin::filesystem;

std::string get_proc_path()
{
    char path[MAX_PATH_STD] = {'\0'};
#if OS_WINDOWS
    DWORD length = GetModuleFileNameA(nullptr, path, MAX_PATH_STD);
    if (length == 0 || length >= MAX_PATH_STD) {
        set_thread_last_err(ERR_COMM_SYSTEM_ERROR);
        ORIGIN_DEBUG_ERR("[WIN32] Get process path failed, length: {}", length);
        length = 0;
    }
#elif OS_LINUX
    auto length = readlink("/proc/self/exe", path, MAX_PATH_STD - 1);
    if (length == -1) {
        set_thread_last_err(ERR_COMM_SYSTEM_ERROR);
        ORIGIN_DEBUG_ERR("[Linux] Get process path failed.");
        length = 0;
    }
    path[length] = '\0';
#elif OS_MACOS
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        set_thread_last_err(ERR_COMM_SYSTEM_ERROR);
        ORIGIN_DEBUG_ERR("[Macos] Get process path failed.");
    }
#else
#error "Unsupport system."
#endif
    set_thread_last_err(ERR_COMM_SUCCESS);
    ORIGIN_DEBUG_TRACE("Get process path: {}", path);
    return path;
}

std::string get_proc_directory()
{
    set_thread_last_err(ERR_COMM_SUCCESS);
    return get_directory(get_proc_path());
}

std::string get_proc_filename_stem()
{
    set_thread_last_err(ERR_COMM_SUCCESS);
    return get_filename_stem(get_proc_path());
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
