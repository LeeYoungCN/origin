/**
 * @file thread_utils.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-08-07
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "origin/thread/thread.h"

#if OS_WINDOWS
#include <windows.h>

#include <processthreadsapi.h>

#elif OS_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#elif OS_MACOS
#include <pthread.h>

#include <cstdint>
#else
#error "Unsupport system"
#endif

#include <string>

#include "origin/common/common_error_code.h"
#include "origin/common/macros/compiler.h"
#include "origin/common/types/error_code_types.h"
#include "origin/common/types/thread_types.h"

namespace {

thread_local std::string g_threadName;

ThreadId GetCurrentThreadIdInternal()
{
#if OS_WINDOWS
    return static_cast<ThreadId>(GetCurrentThreadId());
#elif OS_LINUX
    return static_cast<ThreadId>(syscall(SYS_gettid));
#elif OS_MACOS
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<ThreadId>(tid);
#else
    return 0;
#endif
}
}  // namespace

namespace origin::thread {

ThreadId get_curr_thread_id()
{
    thread_local ThreadId const tid = GetCurrentThreadIdInternal();
    return tid;
}

ErrorCode set_curr_thread_name(const char* name)
{
    g_threadName = name == nullptr ? "" : name;
    return ERR_COMM_SUCCESS;
}

ErrorCode get_curr_thread_name(std::string& out)
{
    out = g_threadName;
    return ERR_COMM_SUCCESS;
}

}  // namespace origin::thread
