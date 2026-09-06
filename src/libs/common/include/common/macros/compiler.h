#ifndef COMMON_MACROS_COMPILER_H
#define COMMON_MACROS_COMPILER_H

// 平台区分宏
#if defined(_WIN32) || defined(_WIN64)
#define OS_WINDOWS 1
#define OS_LINUX   0
#define OS_MACOS   0
#elif defined(__linux__) || defined(linux)
#define OS_WINDOWS 0
#define OS_LINUX   1
#define OS_MACOS   0
#elif defined(__APPLE__)
#define OS_WINDOWS 0
#define OS_LINUX   0
#define OS_MACOS   1
#else
#define OS_WINDOWS 0
#define OS_LINUX   0
#define OS_MACOS   0
#endif

#if defined(_MSC_VER)
#define COMPILER_MSVC  1
#define COMPILER_GCC   0
#define COMPILER_CLANG 0
#define COMPILER_MINGW 0
#elif defined(__MINGW32__) || defined(__MINGW64__)
#define COMPILER_MSVC  0
#define COMPILER_GCC   0
#define COMPILER_CLANG 0
#define COMPILER_MINGW 1
#elif defined(__clang__)
#define COMPILER_MSVC  0
#define COMPILER_GCC   0
#define COMPILER_CLANG 1
#define COMPILER_MINGW 0
#elif defined(__GNUC__)
#define COMPILER_MSVC  0
#define COMPILER_GCC   1
#define COMPILER_CLANG 0
#define COMPILER_MINGW 0
#else
#define COMPILER_MSVC  0
#define COMPILER_GCC   0
#define COMPILER_CLANG 0
#define COMPILER_MINGW 0
#endif

// 编译器版本检测
#if COMPILER_MSVC
#define COMPILER_VERSION _MSC_VER
#elif COMPILER_GCC
#define COMPILER_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#elif COMPILER_CLANG
#define COMPILER_VERSION (__clang_major__ * 100 + __clang_minor__)
#endif

#if defined(__cplusplus) && __cplusplus >= 201103L
#define CPP_STD_11 1
#else
#define CPP_STD_11 0
#endif

// C++标准版本检测（用常量替代defined宏）
#if defined(__cplusplus) && __cplusplus >= 201402L
#define CPP_STD_14 1
#else
#define CPP_STD_14 0
#endif

#if defined(__cplusplus) && __cplusplus >= 201703L
#define CPP_STD_17 1
#else
#define CPP_STD_17 0
#endif

#if defined(__cplusplus) && __cplusplus >= 202002L
#define CPP_STD_20 1
#else
#define CPP_STD_20 0
#endif

// common模块导出宏
#if OS_WINDOWS && COMPILER_MSVC
#ifdef COMMON_BUILD_SHARED
#define COMMON_API __declspec(dllexport)
#else
#define COMMON_API __declspec(dllimport)
#endif
#else
#define COMMON_API __attribute__((visibility("default")))
#endif

// 链接库宏
#ifdef _MSC_VER
#define LINK_LIBRARY(name) __pragma(comment(lib, #name))
#else
#define LINK_LIBRARY(name)
#endif

#endif  // COMMON_MACROS_COMPILER_H
