/**
 * @file version.cpp
 * @brief Utils 组件版本信息实现
 *
 */
#include "origin/utils/version.h"

namespace origin::utils {

std::string_view version() noexcept
{
    return ORIGIN_VERSION_STRING;
}

std::string_view build_time() noexcept
{
    return ORIGIN_BUILD_TIMESTAMP;
}

}  // namespace origin::utils