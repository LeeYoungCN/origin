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

#include "common/macros/compiler.h"

#if OS_WINDOWS
#include <windows.h>
typedef DWORD ThreadId;
#elif OS_LINUX || OS_MACOS
#include <stdint.h>
typedef uint64_t ThreadId;
#else
#include <stdint.h>
typedef uint64_t ThreadId;
#endif

#endif  // COMMON_TYPES_THREAD_TYPES_H
