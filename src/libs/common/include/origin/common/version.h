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
#ifndef COMMON_VERSION_H
#define COMMON_VERSION_H

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

}  // namespace origin

#endif  // COMMON_VERSION_H