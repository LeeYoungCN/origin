/**
 * @file version.cpp
 * @author your name (you@domain.com)
 * @brief 版本信息查询接口实现
 * @version 0.1
 * @date 2025-11-05
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "origin/common/version.h"

#include <format>
#include <string>

namespace origin {

std::string_view version() noexcept
{
    return ORIGIN_VERSION_STRING;
}

std::string version_info()
{
    return std::format("Origin {} (build {})", ORIGIN_VERSION_STRING, ORIGIN_BUILD_TIMESTAMP);
}

bool headers_match_binary() noexcept
{
    return compiled_version().string == version();
}

}  // namespace origin
