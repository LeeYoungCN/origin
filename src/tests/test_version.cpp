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
 * 3) version_info() 里确实带上了版本号与构建时间。
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

TEST(Version, version_info_contains_version_and_build_time)
{
    const std::string info = origin::version_info();
    EXPECT_NE(info.find(std::string(origin::version())), std::string::npos) << info;

    // 构建时间不再暴露成宏（它每次 configure 都会变，见 origin/build_info.h），
    // 这里只校验格式，形如 "(build 2026-11-14T15:34:49+0800)"。
    const std::regex pattern(R"(\(build \d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}[+-]\d{4}\))");
    EXPECT_TRUE(std::regex_search(info, pattern)) << info;
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
    EXPECT_FALSE(info.build_time.empty());

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
    ASSERT_NE(origin_build_time(), nullptr);

    EXPECT_EQ(std::string_view(origin_version()), origin::version());
    EXPECT_EQ(std::string_view(origin_version_commit()), cpp.commit);
    EXPECT_EQ(std::string_view(origin_build_time()), cpp.build_time);
}

}  // namespace origin_test
