/**
 * @file version.h
 * @author your name (you@domain.com)
 * @brief Container 组件自身的版本信息
 * @version 0.1
 * @date 2025-11-05
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#ifndef ORIGIN_CONTAINER_VERSION_H
#define ORIGIN_CONTAINER_VERSION_H

#include <string_view>

#include "origin/version.h"  // 只依赖生成的版本宏，不引入 Common 的头文件

namespace origin::container {
// Container 是纯头文件库，没有二进制，因此只提供编译期接口。

/**
 * @brief 版本字符串，形如 "1.0.0"
 */
[[nodiscard]] constexpr std::string_view version() noexcept
{
    return ORIGIN_VERSION_STRING;
}

/**
 * @brief 构建时间，形如 "2025-11-05T21:34:28+0800"
 */
[[nodiscard]] constexpr std::string_view build_time() noexcept
{
    return ORIGIN_BUILD_TIMESTAMP;
}

[[nodiscard]] constexpr int version_major() noexcept
{
    return ORIGIN_VERSION_MAJOR;
}

[[nodiscard]] constexpr int version_minor() noexcept
{
    return ORIGIN_VERSION_MINOR;
}

[[nodiscard]] constexpr int version_patch() noexcept
{
    return ORIGIN_VERSION_PATCH;
}

}  // namespace origin::container

#endif  // ORIGIN_CONTAINER_VERSION_H