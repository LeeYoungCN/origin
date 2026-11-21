/**
 * @file thread_types.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-08-07
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once

#ifndef COMMON_TYPES_THREAD_TYPES_H
#define COMMON_TYPES_THREAD_TYPES_H

#include "origin/common/macros/compiler.h"

#if OS_WINDOWS
// NOLINTNEXTLINE(misc-include-cleaner): SDK 窄头文件的平台宏由 windows.h 建立
#include <windows.h>

#include <minwindef.h>
typedef DWORD ThreadId;
#elif OS_LINUX || OS_MACOS
#include <stdint.h>
typedef uint64_t ThreadId;
#else
#include <stdint.h>
typedef uint64_t ThreadId;
#endif

#endif  // COMMON_TYPES_THREAD_TYPES_H
