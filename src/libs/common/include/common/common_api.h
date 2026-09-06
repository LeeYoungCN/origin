#ifndef COMMON_COMMON_API_H
#define COMMON_COMMON_API_H
#include "common/macros/compiler.h"

#if COMPILER_MSVC
#ifdef COMMON_BUILD_SHARED
#define COMMON_API __declspec(dllexport)
#else
#define COMMON_API __declspec(dllimport)
#endif
#else
#define COMMON_API __attribute__((visibility("default")))
#endif

#endif  // COMMON_COMMON_API_H
