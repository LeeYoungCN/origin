/**
 * @file commonl.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-29
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef ORIGIN_UTILS_FILESYSTEM_UTILS_INTERNAL_HPP
#define ORIGIN_UTILS_FILESYSTEM_UTILS_INTERNAL_HPP

#include <exception>
#include <system_error>

#include "common/types/error_code_types.h"

namespace origin::filesystem {

ErrorCode ConvertSysEcToErrorCode(const std::error_code& ec);

ErrorCode ConvertExceptionToErrorCode(const std::exception& ex);

}  // namespace origin::filesystem

#endif  // ORIGIN_UTILS_FILESYSTEM_UTILS_INTERNAL_HPP
