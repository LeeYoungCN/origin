#ifndef COMMON_TYPES_PROCESS_TYPES_H
#define COMMON_TYPES_PROCESS_TYPES_H
#include "origin/common/macros/compiler.h"

#if OS_WINDOWS
// 与 DWORD 宽度一致（32 位无符号），但不引入 windows.h，避免污染公共头文件。
typedef unsigned long ProcessId;
#elif OS_LINUX || OS_MACOS
#include <sys/types.h>
typedef pid_t ProcessId;
#else
#include <stdint.h>
typedef uint32_t ProcessId;
#endif
#endif  // COMMON_TYPES_PROCESS_TYPES_H
