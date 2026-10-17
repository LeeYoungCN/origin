#ifndef COMMON_TYPES_PROCESS_TYPES_H
#define COMMON_TYPES_PROCESS_TYPES_H
#include "common/macros/compiler.h"

#if OS_WINDOWS
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
