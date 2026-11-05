/**
 * @file version.h
 * @author your name (you@domain.com)
 * @brief Utils 组件自身的版本信息
 * @version 0.1
 * @date 2025-11-05
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#ifndef ORIGIN_UTILS_VERSION_H
#define ORIGIN_UTILS_VERSION_H

#include <string_view>

#include "origin/common/version.h"  // origin::Version / 版本宏
#include "origin/utils/utils_api.h"

namespace origin::utils {
/**
 * @brief 编译期版本（头文件里的值）
 */
[[nodiscard]] constexpr Version compiled_version() noexcept
{
    return origin::compiled_version();
}

/**
 * @brief 编译期构建时间（头文件里的值）
 */
[[nodiscard]] constexpr std::string_view compiled_build_time() noexcept
{
    return ORIGIN_BUILD_TIMESTAMP;
}

/**
 * @brief 运行期版本（LibOriginUtils 实际编译进去的版本）
 */
UTILS_API std::string_view version() noexcept;

/**
 * @brief 运行期构建时间（LibOriginUtils 实际被构建的时间）
 */
UTILS_API std::string_view build_time() noexcept;

}  // namespace origin::utils

#endif  // ORIGIN_UTILS_VERSION_H