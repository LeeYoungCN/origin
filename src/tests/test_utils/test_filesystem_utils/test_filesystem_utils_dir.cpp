#include "common/macros/compiler.h"
#include "common/types/filesystem_types.h"
#include "gtest/gtest.h"
#include "test_filesystem_utils_base.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

namespace test::test_utils::test_filesystem_utils {

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

    m_testDir1 = to_absolute_path("./test_dir1", m_processDir);
    m_testDir2 = to_absolute_path("./test_dir2", m_testDir1);
    EXPECT_TRUE(delete_dir(m_testDir1, true));
    EXPECT_FALSE(dir_exists(m_testDir1));
}

void TestFilesystemUtilsDir::TearDown()
{
    EXPECT_TRUE(delete_dir(m_testDir1, true));
}

TEST_F(TestFilesystemUtilsDir, DirExists_Exist)
{
    EXPECT_TRUE(dir_exists(m_processDir));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsDir, DirExists_NonExist)
{
    EXPECT_FALSE(dir_exists(m_testDir1));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND);
}

TEST_F(TestFilesystemUtilsDir, DirExists_TargetInvalid)
{
    EXPECT_FALSE(dir_exists(m_process));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_DIRECTORY);
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_NotExist)
{
    EXPECT_TRUE(delete_dir(m_testDir1));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND);
    EXPECT_TRUE(delete_dir(m_process + "/nonexistent"));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND);
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_TypeInvalid)
{
    EXPECT_FALSE(delete_dir(m_process));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_DIRECTORY);
}

TEST_F(TestFilesystemUtilsDir, CreateDir_TypeInvalid)
{
    EXPECT_FALSE(create_dir(m_process));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_DIRECTORY) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsDir, CreateDir_AlreadyExist)
{
    EXPECT_TRUE(create_dir(m_processDir));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_ALREADY_EXISTS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsDir, CreateDir_NotRecursiveSuccess)
{
    EXPECT_TRUE(create_dir(m_testDir1, false));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_FALSE(dir_exists(m_testDir2));
    EXPECT_TRUE(create_dir(m_testDir2, false));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_TRUE(dir_exists(m_testDir2));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsDir, CreateDir_NotRecursiveFalse)
{
    EXPECT_FALSE(create_dir(m_testDir2, false));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND) << get_thread_last_err_msg();
    EXPECT_FALSE(dir_exists(m_testDir2)) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsDir, CreateDir_RecursiveSuccess)
{
    EXPECT_FALSE(dir_exists(m_testDir1));
    EXPECT_TRUE(create_dir(m_testDir2, true));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_TRUE(dir_exists(m_testDir2));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_NotRecursiveFalse)
{
    EXPECT_TRUE(create_dir(m_testDir2, true));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_TRUE(dir_exists(m_testDir2));

    EXPECT_FALSE(delete_dir(m_testDir1, false));
#if COMPILER_MINGW
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_IO_ERROR) << get_thread_last_err_msg();
#else
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_DIR_NOT_EMPTY) << get_thread_last_err_msg();
#endif
    EXPECT_TRUE(dir_exists(m_testDir1));
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_NotRecursiveSuccess)
{
    EXPECT_TRUE(create_dir(m_testDir2, true));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_TRUE(dir_exists(m_testDir2));

    EXPECT_TRUE(delete_dir(m_testDir2, false));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_FALSE(dir_exists(m_testDir2));

    EXPECT_TRUE(delete_dir(m_testDir1, false));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_FALSE(dir_exists(m_testDir1));
}

TEST_F(TestFilesystemUtilsDir, DeleteDir_RecursiveSuccess)
{
    EXPECT_TRUE(create_dir(m_testDir2, true));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_TRUE(dir_exists(m_testDir2));

    EXPECT_TRUE(delete_dir(m_testDir2, true));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_FALSE(dir_exists(m_testDir2));
}

}  // namespace test::test_utils::test_filesystem_utils
