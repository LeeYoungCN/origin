#include "common/common_error_code.h"
#include "container/cstring.hpp"
#include "gtest/gtest.h"

namespace test::test_container::test_CString {

class TestCString : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};
};

TEST_F(TestCString, append)
{
    constexpr uint32_t capacity = 128;
    CString<capacity> testString;
    EXPECT_EQ(testString.length(), 0);
    EXPECT_EQ(testString.capacity(), capacity);
    EXPECT_EQ(testString.last_error(), ERR_COMM_SUCCESS);

    const std::string expectStr = "Hello world!";
    testString.append(expectStr.c_str());
    EXPECT_EQ(testString.last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(testString.length(), expectStr.length());

    testString.reset();
    EXPECT_EQ(testString.length(), 0);
    EXPECT_EQ(testString.capacity(), capacity);
}

TEST_F(TestCString, construct)
{
    constexpr uint32_t capacity = 128;
    const std::string expectStr = "Hello world!";
    CString<capacity> testString = expectStr.c_str();
    EXPECT_EQ(testString.last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(testString.capacity(), capacity);
    EXPECT_EQ(testString.length(), expectStr.length());

    testString.reset();
    EXPECT_EQ(testString.length(), 0);
    EXPECT_EQ(testString.capacity(), capacity);
}

TEST_F(TestCString, overflow)
{
    constexpr uint32_t capacity = 2;
    const std::string expectStr = "Hello world!";
    CString<capacity> testString = expectStr.c_str();
    EXPECT_EQ(testString.last_error(), ERR_CONTAINER_OVERFLOW);
    EXPECT_EQ(testString.capacity(), capacity);
    EXPECT_EQ(testString.length(), 0);
}

}  // namespace test::test_container::test_CString
