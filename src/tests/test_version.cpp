#include <cstring>
#include <regex>
#include <string>
#include <string_view>

#include "gtest/gtest.h"
#include "origin/common/c/version_c.h"
#include "origin/common/version.h"

namespace origin_test {

/**
 * @brief 单版本策略：版本信息只由 Common 提供一套
 *
 * 这里锁住三件事：
 * 1) 编译期宏与 struct 字段一致；
 * 2) 运行期版本非空，且与头文件版本一致；
 * 3) C ABI 的版本号与 getter 与 C++ 侧一致。
 */

TEST(Version, compiled_version_matches_macros)
{
    constexpr auto v = origin::compiled_version();
    EXPECT_EQ(v.major, ORIGIN_VERSION_MAJOR);
    EXPECT_EQ(v.minor, ORIGIN_VERSION_MINOR);
    EXPECT_EQ(v.patch, ORIGIN_VERSION_PATCH);
    EXPECT_EQ(v.string, std::string_view(ORIGIN_VERSION_STRING));
}

TEST(Version, runtime_version_is_not_empty)
{
    EXPECT_FALSE(origin::version().empty());
}

TEST(Version, headers_match_binary)
{
    // 头文件与链接到的库来自同一次构建，必须一致；不一致说明装错了包。
    EXPECT_TRUE(origin::headers_match_binary())
        << "headers=" << origin::compiled_version().string << " binary=" << origin::version();
}


TEST(Version, c_api_abi_version_matches_header)
{
    // 绑定在加载库后应先比对 ABI 版本，避免签名/布局不匹配时直接崩溃
    EXPECT_EQ(origin_c_abi_version(), ORIGIN_C_ABI_VERSION);
}

TEST(Version, build_info_matches_version)
{
    const auto info = origin::build_info();
    EXPECT_EQ(info.version, origin::version());

    if (info.commit.empty()) {
        // 发布构建：完整版本应与头文件里的语义化版本一致
        EXPECT_FALSE(info.dirty);
        EXPECT_EQ(info.version, std::string_view(ORIGIN_VERSION_STRING));
    } else {
        EXPECT_NE(info.version.find(info.commit), std::string_view::npos) << info.version;
    }
}

TEST(Version, header_version_is_plain_semver)
{
    // version.h 只放语义化版本，构建元数据在 build_info.h：
    // 这样每次提交都不会改动 version.h，避免全量重编。
    const std::string_view v = ORIGIN_VERSION_STRING;
    EXPECT_TRUE(std::regex_match(std::string(v), std::regex(R"(\d+\.\d+\.\d+)"))) << v;
    EXPECT_EQ(v.find("-dev"), std::string_view::npos) << v;
}

TEST(Version, c_api_getters_match_cpp)
{
    const auto cpp = origin::build_info();

    ASSERT_NE(origin_version(), nullptr);
    ASSERT_NE(origin_version_commit(), nullptr);

    EXPECT_EQ(std::string_view(origin_version()), origin::version());
    EXPECT_EQ(std::string_view(origin_version_commit()), cpp.commit);
}

}  // namespace origin_test
