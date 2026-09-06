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

#include "utils/thread_utils.h"

#include <string>

#include "common/macros/compiler.h"
#include "common/types/thread_types.h"

#if OS_WINDOWS
#include <windows.h>
#elif OS_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#elif OS_MACOS
#include <pthread.h>
#else
#error "Unsupport system"
#endif

#include "utils/utils_error_code.h"

namespace {

thread_local std::string g_threadName;
thread_local volatile ErrorCode g_lastError = ERR_COMM_SUCCESS;

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
    static thread_local ThreadId tid = GetCurrentThreadIdInternal();
    return tid;
}

void set_curr_thread_name(const char *name)
{
    g_threadName = name;
}

const char *get_curr_thread_name()
{
    return g_threadName.c_str();
}

void set_thread_last_err(ErrorCode errcode)
{
    g_lastError = errcode;
}

ErrorCode get_thread_last_err()
{
    return g_lastError;
}

const char *get_thread_last_err_msg()
{
    return get_utils_err_msg(g_lastError);
}

}  // namespace origin::thread
