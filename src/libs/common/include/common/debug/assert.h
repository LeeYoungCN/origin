/**
 * @file assert.h
 * @author LiYang
 * @brief 公共模块，断言
 * @version 0.1
 * @date 2025-07-22
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#ifndef COMMON_DEBUG_ASSERT_H
#define COMMON_DEBUG_ASSERT_H

#include "common/debug/debug_logger.h"

#ifdef NDEBUG

#ifndef ORIGIN_ASSERT
#define ORIGIN_ASSERT(condition) (static_cast<void>(0))
#endif  // ORIGIN_ASSERT

#ifndef COMMON_ASSERT_MSG
#define COMMON_ASSERT_MSG(condition, msg, ...) (static_cast<void>(0))
#endif  // COMMON_ASSERT_MSG

#else

#ifndef ORIGIN_ASSERT
#define ORIGIN_ASSERT(condition)                          \
    do {                                                  \
        if (!(condition)) {                               \
            ORIGIN_DEBUG_FATAL("assert(" #condition ")"); \
        }                                                 \
    } while (0)
#endif  // ORIGIN_ASSERT

#ifndef COMMON_ASSERT_MSG
#define COMMON_ASSERT_MSG(condition, fmt, ...)                  \
    do {                                                        \
        if (!(condition)) {                                     \
            ORIGIN_DEBUG_FATAL(fmt __VA_OPT__(, ) __VA_ARGS__); \
        }                                                       \
    } while (0)
#endif  // COMMON_ASSERT_MSG
#endif  // NDEBUG
#endif  // COMMON_DEBUG_ASSERT_H
