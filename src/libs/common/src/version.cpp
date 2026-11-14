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

#include <string>
#include <string_view>

#include "origin/build_info.h"

namespace origin {

std::string_view version() noexcept
{
    // 开发版返回完整串（含 -dev+g<sha>），发布构建与头文件里的语义化版本相同
    return ORIGIN_VERSION_FULL;
}

BuildInfo build_info() noexcept
{
    // 字段都由 configure 阶段生成，避免在运行期解析版本字符串
    //（解析出来的子串不是 NUL 结尾，C 接口会把它后面的 "-dirty" 一起读出去）。
    BuildInfo info{};
    info.version = ORIGIN_VERSION_FULL;
    info.commit = ORIGIN_VERSION_COMMIT;
    info.dirty = ORIGIN_VERSION_DIRTY != 0;
    return info;
}

bool headers_match_binary() noexcept
{
    // 头文件里是语义化版本（1.0.1），运行期可能是开发版完整串（1.0.1-dev+g...）；
    // 判据：运行时版本以头文件版本为前缀，且后面紧跟结尾或 '-'
    const std::string_view header_version = compiled_version().string;
    const std::string_view runtime_version = version();
    if (runtime_version.size() < header_version.size() ||
        runtime_version.substr(0, header_version.size()) != header_version) {
        return false;
    }
    return runtime_version.size() == header_version.size() || runtime_version[header_version.size()] == '-';
}

}  // namespace origin
