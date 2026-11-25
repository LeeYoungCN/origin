#ifndef ORIGIN_LOGGING_INTERNAL_COMMON_HPP
#define ORIGIN_LOGGING_INTERNAL_COMMON_HPP

#include <cstdint>
#include <string>
#include <string_view>

#include "origin/common/types/error_code_types.h"

namespace origin::logging {
[[nodiscard]] ErrorCode get_default_log_file(std::string_view suffix, std::string& out);
std::string get_default_log_file(std::string_view suffix = "log");

bool delete_file_retry(std::string_view file, uint32_t maxRetry, uint32_t sleepMs);

bool rename_file_retry(std::string_view src,
                       std::string_view dest,
                       bool overwrite,
                       uint32_t maxRetry,
                       uint32_t sleepMs);

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_INTERNAL_COMMON_HPP
