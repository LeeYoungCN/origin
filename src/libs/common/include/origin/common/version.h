/**
 * @file version.h
 * @author your name (you@domain.com)
 * @brief 版本信息查询接口
 * @version 0.1
 * @date 2025-11-05
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#ifndef ORIGIN_COMMON_VERSION_H
#define ORIGIN_COMMON_VERSION_H

#include <string_view>

#include "origin/common/common_api.h"
#include "origin/version.h"  // 由 cmake/version.h.in 在 configure 阶段生成

namespace origin {
/**
 * @brief 版本三元组与完整版本字符串
 */
struct Version
{
    int major = 0;              ///< 主版本号
    int minor = 0;              ///< 次版本号
    int patch = 0;              ///< 修订号
    std::string_view string{};  ///< 完整版本，形如 "1.0.0"
};

/**
 * @brief 编译期版本（来自当前编译单元引用的头文件）
 *
 * 适合 static_assert、编译期分支等场景。
 */
[[nodiscard]] constexpr Version compiled_version() noexcept
{
    return Version{ORIGIN_VERSION_MAJOR, ORIGIN_VERSION_MINOR, ORIGIN_VERSION_PATCH, ORIGIN_VERSION_STRING};
}

/**
 * @brief 运行期版本（库内部实际编译进去的版本）
 *
 * 只替换了 so/dll 而没有重新编译调用方时，返回值与 compiled_version() 可能不同，
 * 排查版本不一致时以这个为准。
 */
COMMON_API std::string_view version() noexcept;

/**
 * @brief 编译期构建时间（头文件里的值，形如 "2025-11-05T21:34:28+0800"）
 */
[[nodiscard]] constexpr std::string_view compiled_build_time() noexcept
{
    return ORIGIN_BUILD_TIMESTAMP;
}

/**
 * @brief 运行期构建时间（库实际被构建的时间）
 */
COMMON_API std::string_view build_time() noexcept;

}  // namespace origin

/**
 * @brief Common 组件自身的版本信息
 *
 * 最早的 origin::version() 就实现在 Common 里，这里补一个同构的 origin::common::xxx 入口，
 * 让每个组件都能用 origin::<component>::version() 的统一写法。
 */
namespace origin::common {
[[nodiscard]] constexpr Version compiled_version() noexcept
{
    return origin::compiled_version();
}

[[nodiscard]] constexpr std::string_view compiled_build_time() noexcept
{
    return origin::compiled_build_time();
}

inline std::string_view version() noexcept
{
    return origin::version();
}

inline std::string_view build_time() noexcept
{
    return origin::build_time();
}
}  // namespace origin::common

#endif  // ORIGIN_COMMON_VERSION_H
