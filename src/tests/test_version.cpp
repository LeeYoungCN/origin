#include <cstring>
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
    EXPECT_NE(info.find(std::string_view(ORIGIN_BUILD_TIMESTAMP)), std::string::npos) << info;
}

TEST(Version, c_api_version_matches_cpp_api)
{
    ASSERT_NE(origin_version(), nullptr);
    EXPECT_EQ(std::string_view(origin_version()), origin::version());
}

TEST(Version, c_api_version_info_roundtrip)
{
    const size_t need = origin_version_info(nullptr, 0);
    ASSERT_GT(need, 0u);

    std::string buf(need + 1, '\0');
    EXPECT_EQ(origin_version_info(buf.data(), buf.size()), need);
    EXPECT_EQ(std::string(buf.c_str()), origin::version_info());
}

TEST(Version, c_api_version_info_truncates_when_buffer_too_small)
{
    std::string buf(8, '\0');
    const size_t written = origin_version_info(buf.data(), buf.size());
    EXPECT_EQ(written, buf.size() - 1);
    EXPECT_EQ(std::strlen(buf.c_str()), buf.size() - 1);
}

}  // namespace origin_test
