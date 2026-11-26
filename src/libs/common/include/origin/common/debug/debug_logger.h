#ifndef COMMON_DEBUG_DEBUG_LOGGER_H
#define COMMON_DEBUG_DEBUG_LOGGER_H

#include <string>

#include "origin/common/common_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>

#define ORIGIN_DBG_LEVEL_TRACE 0
#define ORIGIN_DBG_LEVEL_DEBUG 1
#define ORIGIN_DBG_LEVEL_INFO  2
#define ORIGIN_DBG_LEVEL_WARN  3
#define ORIGIN_DBG_LEVEL_ERR   4
#define ORIGIN_DBG_LEVEL_FATAL 5
#define ORIGIN_DBG_LEVEL_OFF   6

typedef enum {
    ORG_DBG_LVL_TRACE = ORIGIN_DBG_LEVEL_TRACE,
    ORG_DBG_LVL_DEBUG = ORIGIN_DBG_LEVEL_DEBUG,
    ORG_DBG_LVL_INFO = ORIGIN_DBG_LEVEL_INFO,
    ORG_DBG_LVL_WARN = ORIGIN_DBG_LEVEL_WARN,
    ORG_DBG_LVL_ERR = ORIGIN_DBG_LEVEL_ERR,
    ORG_DBG_LVL_FATAL = ORIGIN_DBG_LEVEL_FATAL,
    ORG_DBG_LVL_OFF = ORIGIN_DBG_LEVEL_OFF
} OriginDbgLvl;

COMMON_API OriginDbgLvl origin_get_debug_logger_level();

COMMON_API void origin_set_debug_logger_level(OriginDbgLvl level);

COMMON_API bool origin_debug_logger_should_log(OriginDbgLvl level);

COMMON_API void origin_debug_logger_c(
    const char* file, int line, const char* func, OriginDbgLvl level, const char* format, ...);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <format>
COMMON_API void origin_debug_logger_log(
    const char* file, int line, const char* func, OriginDbgLvl level, const std::string& message);

template <typename... Args>
void origin_debug_logger_log(const char* file,
                             int line,
                             const char* func,
                             OriginDbgLvl level,
                             std::format_string<Args...> format,
                             Args&&... args)
{
    if (origin_debug_logger_should_log(level)) {
        origin_debug_logger_log(
            file, line, func, level, std::move(std::format(format, std::forward<Args>(args)...)));
    }
}
#endif

/*
 * Debug 日志的编译期最低级别：
 *   - Debug/RelWithDebInfo：TRACE 全开
 *   - Release 默认：ERR/FATAL
 *   - ORIGIN_DEBUG_LOG_ALL_LEVELS=ON：编译全部级别（含 Release）
 *
 */
#ifndef ORIGIN_DEBUG_LOG_ALL_LEVELS
#define ORIGIN_DEBUG_LOG_ALL_LEVELS 0
#endif

#if ORIGIN_BUILD_DEBUG || ORIGIN_BUILD_RELWITHDEBINFO || ORIGIN_DEBUG_LOG_ALL_LEVELS
#define ORIGIN_DEBUG_LOG_MIN_LEVEL ORIGIN_DBG_LEVEL_TRACE
#else
#define ORIGIN_DEBUG_LOG_MIN_LEVEL ORIGIN_DBG_LEVEL_ERR
#endif

#ifdef __cplusplus
#define ORIGIN_DEBUG_NOOP (static_cast<void>(0))
#else
#define ORIGIN_DEBUG_NOOP ((void)0)
#endif

#ifdef __cplusplus
#define ORIGIN_DEBUG_LOGGER(level, format, ...)                                          \
    do {                                                                                 \
        origin_debug_logger_log(                                                         \
            __FILE__, __LINE__, __FUNCTION__, level, format __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)
#else
#define ORIGIN_DEBUG_LOGGER(level, format, ...)                                          \
    do {                                                                                 \
        origin_debug_logger_c(                                                           \
            __FILE__, __LINE__, __FUNCTION__, level, format __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)
#endif

#if ORIGIN_DEBUG_LOG_MIN_LEVEL <= ORIGIN_DBG_LEVEL_TRACE
#define ORIGIN_DEBUG_TRACE(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_TRACE, format __VA_OPT__(, ) __VA_ARGS__);
#else
#define ORIGIN_DEBUG_TRACE(format, ...) ORIGIN_DEBUG_NOOP;
#endif

#if ORIGIN_DEBUG_LOG_MIN_LEVEL <= ORIGIN_DBG_LEVEL_DEBUG
#define ORIGIN_DEBUG_DBG(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_DEBUG, format __VA_OPT__(, ) __VA_ARGS__);
#else
#define ORIGIN_DEBUG_DBG(format, ...) ORIGIN_DEBUG_NOOP;
#endif

#if ORIGIN_DEBUG_LOG_MIN_LEVEL <= ORIGIN_DBG_LEVEL_INFO
#define ORIGIN_DEBUG_INFO(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_INFO, format __VA_OPT__(, ) __VA_ARGS__);
#else
#define ORIGIN_DEBUG_INFO(format, ...) ORIGIN_DEBUG_NOOP;
#endif

#if ORIGIN_DEBUG_LOG_MIN_LEVEL <= ORIGIN_DBG_LEVEL_WARN
#define ORIGIN_DEBUG_WARN(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_WARN, format __VA_OPT__(, ) __VA_ARGS__);
#else
#define ORIGIN_DEBUG_WARN(format, ...) ORIGIN_DEBUG_NOOP;
#endif

#if ORIGIN_DEBUG_LOG_MIN_LEVEL <= ORIGIN_DBG_LEVEL_ERR
#define ORIGIN_DEBUG_ERR(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_ERR, format __VA_OPT__(, ) __VA_ARGS__);
#else
#define ORIGIN_DEBUG_ERR(format, ...) ORIGIN_DEBUG_NOOP;
#endif

#if ORIGIN_DEBUG_LOG_MIN_LEVEL <= ORIGIN_DBG_LEVEL_FATAL
#define ORIGIN_DEBUG_FATAL(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_FATAL, format __VA_OPT__(, ) __VA_ARGS__);
#else
#define ORIGIN_DEBUG_FATAL(format, ...) ORIGIN_DEBUG_NOOP;
#endif

#endif  // COMMON_DEBUG_DEBUG_LOGGER_H
