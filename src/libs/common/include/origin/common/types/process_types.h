#ifndef COMMON_TYPES_PROCESS_TYPES_H
#define COMMON_TYPES_PROCESS_TYPES_H
#include "origin/common/macros/compiler.h"

#if OS_WINDOWS
// NOLINTNEXTLINE(misc-include-cleaner): SDK 窄头文件的平台宏由 windows.h 建立
#include <windows.h>

#include <minwindef.h>
typedef DWORD ProcessId;
#elif OS_LINUX || OS_MACOS
#include <sys/types.h>
typedef pid_t ProcessId;
#else
#include <stdint.h>
typedef uint32_t ProcessId;
#endif
#endif  // COMMON_TYPES_PROCESS_TYPES_H
