#ifndef ORIGIN_LOGGING_INTERNAL_COMMON_HPP
#define ORIGIN_LOGGING_INTERNAL_COMMON_HPP

#include <cstdint>
#include <string>

#include "common/debug/debug_logger.h"

#define RETURN_AND_LOG_IF_PTR_NULL(ptr, log)        \
    if (!(ptr)) {                                   \
        ORIGIN_DEBUG_ERR(log " " #ptr " nullptr."); \
        return;                                     \
    }

#define RETURN_VALUE_IF_PTR_NULL(ptr, value) \
    if (!(ptr)) {                            \
        ORIGIN_DEBUG_ERR(#ptr " nullptr.");  \
        return value;                        \
    }

#define RETURN_IF_TRUE(condition) \
    if ((condition)) {            \
        return;                   \
    }

#define RETURN_AND_ERROR_IF_TRUE(condition, format, ...)     \
    if ((condition)) {                                       \
        ORIGIN_DEBUG_ERR(format __VA_OPT__(, ) __VA_ARGS__); \
        return;                                              \
    }

#define RETURN_AND_WARN_IF_TRUE(condition, format, ...)       \
    if ((condition)) {                                        \
        ORIGIN_DEBUG_WARN(format __VA_OPT__(, ) __VA_ARGS__); \
        return;                                               \
    }

#define RETURN_VALUE_IF_TRUE(condition, value) \
    if ((condition)) {                         \
        return value;                          \
    }

#define RETURN_VALUE_AND_WARN_IF_TRUE(condition, value, format, ...) \
    if ((condition)) {                                               \
        ORIGIN_DEBUG_WARN(format __VA_OPT__(, ) __VA_ARGS__);        \
        return value;                                                \
    }

#define RETURN_VALUE_AND_ERROR_IF_TRUE(condition, value, format, ...) \
    if ((condition)) {                                                \
        ORIGIN_DEBUG_ERR(format __VA_OPT__(, ) __VA_ARGS__);          \
        return value;                                                 \
    }

namespace origin::logging {
std::string get_default_log_file(std::string_view suffix = "log");

bool delete_file(std::string_view file, uint32_t maxRetry, uint32_t sleepMs);

bool rename_file(std::string_view src, std::string_view dest, bool overwrite, uint32_t maxRetry,
                 uint32_t sleepMs);

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_INTERNAL_COMMON_HPP
