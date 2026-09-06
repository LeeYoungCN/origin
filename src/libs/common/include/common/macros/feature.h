#ifndef COMMON_COMPILER_FEATURE_H
#define COMMON_COMPILER_FEATURE_H

#include "common/macros/compiler.h"

// 检测是否支持thread_local（C++11特性）
#if CPP_STD_11 && (COMPILER_MSVC || COMPILER_GCC || COMPILER_CLANG)
#define SUPPORTS_THREAD_LOCAL 1
#else
#define SUPPORTS_THREAD_LOCAL 0
#endif

// 检测是否支持constexpr（C++11及以上）
#if CPP_STD_11
#define SUPPORTS_CONSTEXPR 1
#else
#define SUPPORTS_CONSTEXPR 0
#endif

// 检测是否支持结构化绑定（C++17特性）
#if CPP_STD_17
#define SUPPORTS_STRUCTURED_BINDINGS 1
#else
#define SUPPORTS_STRUCTURED_BINDINGS 0
#endif

// 检测是否支持折叠表达式（C++17特性）
#if CPP_STD_17
#define SUPPORTS_FOLD_EXPRESSIONS 1
#else
#define SUPPORTS_FOLD_EXPRESSIONS 0
#endif

// 检测是否支持 Concepts（C++20特性）
#if CPP_STD_20 && (COMPILER_GCC >= 1000 || COMPILER_CLANG >= 1000 || COMPILER_MSVC >= 1928)
#define SUPPORTS_CONCEPTS 1
#else
#define SUPPORTS_CONCEPTS 0
#endif

// 特性兼容宏：如果不支持，提供降级实现
#if !SUPPORTS_THREAD_LOCAL
// 不支持thread_local时，用pthread_key_t模拟（仅示例框架）
#define thread_local __thread  // 对GCC旧版本的兼容
#endif

#endif  // COMMON_COMPILER_FEATURE_H
