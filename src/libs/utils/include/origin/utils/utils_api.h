/**
 * @file utils_api.h
 * @brief Utils 组件的导出宏
 *
 */
#ifndef ORIGIN_UTILS_UTILS_API_H
#define ORIGIN_UTILS_UTILS_API_H

#include "origin/common/macros/compiler.h"

// 默认编译动态库
#ifdef UTILS_STATIC
#define UTILS_API
#else  // UTILS_STATIC
#if OS_WINDOWS && COMPILER_MSVC
#ifdef UTILS_BUILD_SHARED
#define UTILS_API __declspec(dllexport)
#else  // UTILS_BUILD_SHARED
#define UTILS_API __declspec(dllimport)
#endif  // UTILS_BUILD_SHARED
#else   // OS_WINDOWS && COMPILER_MSVC
#define UTILS_API __attribute__((visibility("default")))
#endif  // OS_WINDOWS && COMPILER_MSVC
#endif  // UTILS_STATIC

#endif  // ORIGIN_UTILS_UTILS_API_H