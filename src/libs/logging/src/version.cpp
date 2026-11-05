/**
 * @file version.cpp
 * @brief Logging 组件版本信息实现
 *
 */
#include "origin/logging/version.h"

namespace origin::logging {

std::string_view version() noexcept
{
    return ORIGIN_VERSION_STRING;
}

std::string_view build_time() noexcept
{
    return ORIGIN_BUILD_TIMESTAMP;
}

}  // namespace origin::logging