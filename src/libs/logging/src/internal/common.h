#ifndef ORIGIN_LOGGING_INTERNAL_COMMON_H
#define ORIGIN_LOGGING_INTERNAL_COMMON_H

#include <cstdint>
#include <string>

#include "common/debug/debug_logger.h"
#include "logging/log_level.h"

#define RETURN_IF_PTR_NULL(ptr)             \
    if (!(ptr)) {                           \
        ORIGIN_DEBUG_ERR(#ptr " nullptr."); \
        return;                             \
    }

#define RETURN_VALUE_IF_PTR_NULL(ptr, value) \
    if (!(ptr)) {                            \
        ORIGIN_DEBUG_ERR(#ptr " nullptr.");  \
        return value;                        \
    }

namespace origin::logging {
std::string_view log_level_to_string(LogLevel level, bool full = true);

std::string get_default_log_file(std::string_view suffix = "log");

bool delete_file(std::string_view file, uint32_t maxRetry, uint32_t sleepMs);

bool rename_file(std::string_view src, std::string_view dest, bool overwrite, uint32_t maxRetry,
                 uint32_t sleepMs);

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_INTERNAL_COMMON_H
