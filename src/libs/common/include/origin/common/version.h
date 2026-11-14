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

#include <string>
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
 * @brief 结构化的构建信息
 *
 * 适合需要把版本 / 提交 / 构建时间分开上报的场景（崩溃报告、遥测、JSON 输出），
 * 调用方不必解析 version_info() 的字符串。
 */
struct BuildInfo
{
    std::string_view version;     ///< 完整版本，形如 "1.0.1-dev+g7b4ef62"
    std::string_view commit;      ///< 短 git sha；发布构建或无 git 时为空
    bool dirty = false;           ///< 开发版构建时工作区是否有未提交修改
    std::string_view build_time;  ///< configure 时间，形如 "2026-11-14T12:11:59+0800"
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
 * @brief 版本信息汇总，供日志 / --version / 崩溃报告直接打印
 *
 * 形如 "Origin 1.0.1-dev+g7b4ef62 (build 2026-11-14T12:11:59+0800)"。
 * 返回 std::string 而不是 string_view，避免静态缓冲区带来的线程与生命周期问题。
 */
COMMON_API std::string version_info();

/**
 * @brief 获取结构化的构建信息（版本 / 提交 / 是否脏 / 构建时间）
 */
[[nodiscard]] COMMON_API BuildInfo build_info() noexcept;

/**
 * @brief 头文件版本与运行期库版本是否一致
 *
 * 不一致通常意味着"装了 A 版本的头文件、链接的是 B 版本的库"：
 * Linux 上不会报错但可能 ABI 不兼容，建议启动时断言或打一条日志。
 */
[[nodiscard]] COMMON_API bool headers_match_binary() noexcept;

}  // namespace origin

#endif  // ORIGIN_COMMON_VERSION_H
