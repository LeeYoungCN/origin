/**
 * @file process_types.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2026-02-05
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#ifndef COMMON_TYPES_PROCESS_TYPES_H
#define COMMON_TYPES_PROCESS_TYPES_H
#include "common/macros/compiler.h"

#if OS_WINDOWS
#include <windows.h>
typedef DWORD ProcessId;
#elif OS_LINUX || OS_MACOS
#include <sys/types.h>
typedef pid_t ProcessId;
#else
#include <stdint.h>
typedef uint32_t ProcessId;
#endif
#endif  // COMMON_TYPES_PROCESS_TYPES_H
