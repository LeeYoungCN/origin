#ifndef COMMON_DEBUG_DEBUG_LOGGER_H
#define COMMON_DEBUG_DEBUG_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>

typedef enum {
    ORG_DBG_LVL_TRACE = 0,
    ORG_DBG_LVL_DEBUG,
    ORG_DBG_LVL_INFO,
    ORG_DBG_LVL_WARN,
    ORG_DBG_LVL_ERR,
    ORG_DBG_LVL_FATAL,
    ORG_DBG_LVL_OFF
} OriginDbgLvl;

void origin_set_debug_logger_level(OriginDbgLvl level);

bool origin_debug_logger_should_log(OriginDbgLvl level);

void origin_debug_logger_c(const char* file, int line, const char* func, OriginDbgLvl level,
                           const char* format, ...);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <format>
void origin_debug_logger_log(const char* file, int line, const char* func, OriginDbgLvl level,
                             const std::string& message);

template <typename... Args>
void origin_debug_logger_log(const char* file, int line, const char* func, OriginDbgLvl level,
                             std::format_string<Args...> format, Args&&... args)
{
    if (origin_debug_logger_should_log(level)) {
        origin_debug_logger_log(
            file, line, func, level, std::move(std::format(format, std::forward<Args>(args)...)));
    }
}
#endif

#if defined(NDEBUG) && !defined(ENABLE_TEST)
// Release模式：空操作，显式消费所有参数避免警告
#define ORIGIN_DEBUG_LOGGER(level, format, ...) (static_cast<void>(0))
#else
#ifdef __cplusplus
// Debug模式：实际日志输出，支持所有级别
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
#endif

#define ORIGIN_DEBUG_TRACE(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_TRACE, format __VA_OPT__(, ) __VA_ARGS__);

#define ORIGIN_DEBUG_DBG(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_DEBUG, format __VA_OPT__(, ) __VA_ARGS__);

#define ORIGIN_DEBUG_INFO(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_INFO, format __VA_OPT__(, ) __VA_ARGS__);

#define ORIGIN_DEBUG_WARN(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_WARN, format __VA_OPT__(, ) __VA_ARGS__);

#define ORIGIN_DEBUG_ERR(format, ...) \
    ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_ERR, format __VA_OPT__(, ) __VA_ARGS__);

#define ORIGIN_DEBUG_FATAL(format, ...)                                            \
    do {                                                                           \
        ORIGIN_DEBUG_LOGGER(ORG_DBG_LVL_FATAL, format __VA_OPT__(, ) __VA_ARGS__); \
        std::abort();                                                              \
    } while (0)

#endif  // COMMON_DEBUG_DEBUG_LOG_HPP
