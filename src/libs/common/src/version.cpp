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

namespace origin {

std::string_view version() noexcept
{
    return ORIGIN_VERSION_STRING;
}

std::string_view build_time() noexcept
{
    return ORIGIN_BUILD_TIMESTAMP;
}

}  // namespace origin
