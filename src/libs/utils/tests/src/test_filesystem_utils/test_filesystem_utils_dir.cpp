#include <string>

#include "gtest/gtest.h"
#include "origin/common/common_error_code.h"
#include "origin/common/macros/compiler.h"
#include "origin/filesystem/filesystem.h"
#include "test_filesystem_utils_base.h"

namespace utils_test::filesystem_test {
using namespace origin::thread;
using namespace origin::filesystem;

class TestFilesystemUtilsDir : public TestFilesystemUtilsBase {
protected:
    void SetUp() override;
    void TearDown() override;

protected:
    std::string m_testDir1;
    std::string m_testDir2;
};

void TestFilesystemUtilsDir::SetUp()
{
    TestFilesystemUtilsBase::SetUp();

    m_testDir1 = fs_abs("./test_dir1_" + std::to_string(m_pid), m_processDir);
    m_testDir2 = fs_abs("./test_dir2_" + std::to_string(m_pid), m_testDir1);
    EXPECT_EQ(delete_dir(m_testDir1, true), ERR_COMM_SUCCESS);
    EXPECT_FALSE(dir_exists(m_testDir1));
}

void TestFilesystemUtilsDir::TearDown()
{
    EXPECT_EQ(delete_dir(m_testDir1, true), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsDir, DirExists_Exist)
{
    EXPECT_TRUE(dir_exists(m_processDir));
}

TEST_F(TestFilesystemUtilsDir, DirExists_NonExist)
{
    EXPECT_FALSE(dir_exists(m_testDir1));
}

TEST_F(TestFilesystemUtilsDir, DirExists_TargetInvalid)
{
    EXPECT_FALSE(dir_exists(m_process));
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_NotExist)
{
    EXPECT_EQ(delete_dir(m_testDir1), ERR_COMM_SUCCESS);
    EXPECT_EQ(delete_dir(m_process + "/nonexistent"), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_TypeInvalid)
{
    EXPECT_NE(delete_dir(m_process), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsDir, CreateDir_TypeInvalid)
{
    EXPECT_NE(create_dir(m_process), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsDir, CreateDir_AlreadyExist)
{
    EXPECT_EQ(create_dir(m_processDir), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsDir, CreateDir_NotRecursiveSuccess)
{
    EXPECT_EQ(create_dir(m_testDir1, false), ERR_COMM_SUCCESS);
    EXPECT_FALSE(dir_exists(m_testDir2));
    EXPECT_EQ(create_dir(m_testDir2, false), ERR_COMM_SUCCESS);
    EXPECT_TRUE(dir_exists(m_testDir2));
}

TEST_F(TestFilesystemUtilsDir, CreateDir_NotRecursiveFalse)
{
    EXPECT_NE(create_dir(m_testDir2, false), ERR_COMM_SUCCESS);
    EXPECT_FALSE(dir_exists(m_testDir2));
}

TEST_F(TestFilesystemUtilsDir, CreateDir_RecursiveSuccess)
{
    EXPECT_FALSE(dir_exists(m_testDir1));
    EXPECT_EQ(create_dir(m_testDir2, true), ERR_COMM_SUCCESS);
    EXPECT_TRUE(dir_exists(m_testDir2));
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_NotRecursiveFalse)
{
    EXPECT_EQ(create_dir(m_testDir2, true), ERR_COMM_SUCCESS);
    EXPECT_TRUE(dir_exists(m_testDir2));

    EXPECT_NE(delete_dir(m_testDir1, false), ERR_COMM_SUCCESS);
#if COMPILER_MINGW
#else
#endif
    EXPECT_TRUE(dir_exists(m_testDir1));
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_NotRecursiveSuccess)
{
    EXPECT_EQ(create_dir(m_testDir2, true), ERR_COMM_SUCCESS);
    EXPECT_TRUE(dir_exists(m_testDir2));

    EXPECT_EQ(delete_dir(m_testDir2, false), ERR_COMM_SUCCESS);
    EXPECT_FALSE(dir_exists(m_testDir2));

    EXPECT_EQ(delete_dir(m_testDir1, false), ERR_COMM_SUCCESS);
    EXPECT_FALSE(dir_exists(m_testDir1));
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_RecursiveSuccess)
{
    EXPECT_EQ(create_dir(m_testDir2, true), ERR_COMM_SUCCESS);
    EXPECT_TRUE(dir_exists(m_testDir2));

    EXPECT_EQ(delete_dir(m_testDir2, true), ERR_COMM_SUCCESS);
    EXPECT_FALSE(dir_exists(m_testDir2));
}

}  // namespace utils_test::filesystem_test
