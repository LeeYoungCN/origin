#ifndef ORIGIN_LOGGING_DETAIL_LOGGING_API_H
#define ORIGIN_LOGGING_DETAIL_LOGGING_API_H

#include "common/macros/compiler.h"

// 默认编译动态库
#ifdef LOGGING_STATIC
#define LOGGING_API
#else  // LOGGING_STATIC
#if OS_WINDOWS && COMPILER_MSVC
#ifdef LOGGING_BUILD_SHARED
#define LOGGING_API __declspec(dllexport)
#else  // LOGGING_BUILD_SHARED
#define LOGGING_API __declspec(dllimport)
#endif  // LOGGING_BUILD_SHARED
#else   // OS_WINDOWS && COMPILER_MSVC
#define LOGGING_API __attribute__((visibility("default")))
#endif  // OS_WINDOWS && COMPILER_MSVC
#endif  // LOGGING_STATIC

#endif  //  ORIGIN_LOGGING_DETAIL_LOGGING_API_H
