#ifndef COMMON_COMPILER_WARNINGS_H
#define COMMON_COMPILER_WARNINGS_H

#include "common/macros/compiler.h"

// 通用警告设置：开启严格警告（工程推荐）
#if COMPILER_MSVC
// MSVC：开启所有警告，视为错误
#pragma warning(push, 4)         // 最高警告级别
#pragma warning(disable : 4100)  // 屏蔽"未使用参数"警告（调试常用）
#pragma warning(disable : 4996)  // 屏蔽"函数已废弃"警告（如strcpy）
#elif COMPILER_GCC || COMPILER_CLANG
// GCC/Clang：开启所有警告，视为错误
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic ignored "-Wunused-parameter"         // 屏蔽"未使用参数"警告
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"  // 屏蔽"废弃声明"警告
#endif

// 特定场景的警告控制宏
// 示例1：屏蔽"未使用变量"警告（用于必须定义但暂时不用的变量）
#define UNUSED(var) (void)(var)

// 示例2：屏蔽"类型转换可能丢失数据"警告（确认安全时使用）
#if COMPILER_MSVC
#define SUPPRESS_LOSSY_CONVERSION_WARNING __pragma(warning(suppress : 4244))
#elif COMPILER_GCC || COMPILER_CLANG
#define SUPPRESS_LOSSY_CONVERSION_WARNING _Pragma("GCC diagnostic ignored \"-Wconversion\"")
#endif

#endif  // COMMON_COMPILER_WARNINGS_H
