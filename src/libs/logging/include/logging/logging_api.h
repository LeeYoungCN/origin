#ifndef ORIGIN_LOGGING_LOGGING_API_H
#define ORIGIN_LOGGING_LOGGING_API_H

#include "common/macros/compiler.h"

#if COMPILER_MSVC
#ifdef LOGGING_BUILD_SHARED
#define LOGGING_API __declspec(dllexport)
#else
#define LOGGING_API __declspec(dllimport)
#endif
#else
#define LOGGING_API __attribute__((visibility("default")))
#endif

#endif  //  ORIGIN_LOGGING_LOGGING_API_H
