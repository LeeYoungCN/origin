/**
 * @file test_filesystem_utils_file.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-30
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "gtest/gtest.h"

#include <string>

#include "origin/common/common_error_code.h"
#include "origin/common/constants/filesystem_constants.h"
#include "origin/common/macros/compiler.h"

#include "origin/filesystem/filesystem.h"

#include "test_filesystem_utils_base.h"

namespace utils_test::filesystem_test {
using namespace origin::thread;
using namespace origin::filesystem;

class TestFilesystemUtilsFile : public TestFilesystemUtilsBase {
protected:
    void SetUp() override;
    void TearDown() override;

protected:
    std::string m_testFile;
    std::string m_testFilename{"test_file"};
};

void TestFilesystemUtilsFile::SetUp()
{
    TestFilesystemUtilsBase::SetUp();
    m_testFile = fs_abs(m_testFilename, m_processDir);
}

void TestFilesystemUtilsFile::TearDown()
{
    TestFilesystemUtilsBase::TearDown();
    EXPECT_EQ(delete_file(m_testFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, file_exists)
{
    EXPECT_TRUE(file_exists(m_process));
}

TEST_F(TestFilesystemUtilsFile, file_exists_nonexistent)
{
    EXPECT_FALSE(file_exists(m_testFile));
}

TEST_F(TestFilesystemUtilsFile, file_exists_type_invalid)
{
    EXPECT_FALSE(file_exists(m_processDir));
}

TEST_F(TestFilesystemUtilsFile, create_file_already_exists)
{
    EXPECT_EQ(create_file(m_process), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, create_file_type_invalid)
{
    EXPECT_NE(create_file(m_processDir), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, create_file_success)
{
    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, create_file_dir_nonexistent)
{
    auto newFile = fs_abs(m_testFilename, m_processDir + "/nonexistent");
    EXPECT_NE(create_file(newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, delete_file_dir_nonexistent)
{
    auto newFile = fs_abs(m_testFilename, m_processDir + "/nonexistent");
    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, delete_file_type_invalid)
{
    EXPECT_NE(delete_file(m_processDir), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, delete_file_file_exist)
{
    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(delete_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(delete_file(m_testFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, read_file_success)
{
    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(fs_read_text(m_testFile), "");
    EXPECT_EQ(fs_size(m_testFile), 0);
}

TEST_F(TestFilesystemUtilsFile, read_file_target_invalid)
{
    EXPECT_EQ(fs_read_text(m_processDir), "");
}

TEST_F(TestFilesystemUtilsFile, read_file_nonexistent)
{
    EXPECT_EQ(fs_read_text(m_testFile), "");
}

TEST_F(TestFilesystemUtilsFile, write_text_file_success)
{
    std::string text = "1234";
    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(write_text_file(m_testFile, text), ERR_COMM_SUCCESS);
    EXPECT_EQ(fs_read_text(m_testFile), text);
    EXPECT_EQ(fs_size(m_testFile), text.size());
}

TEST_F(TestFilesystemUtilsFile, write_text_file_overwrite_success)
{
    std::string text = "1234";
    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);

    EXPECT_EQ(write_text_file(m_testFile, text, true), ERR_COMM_SUCCESS);
    EXPECT_EQ(fs_read_text(m_testFile), text);
    EXPECT_EQ(fs_size(m_testFile), text.size());

    EXPECT_EQ(write_text_file(m_testFile, text, true), ERR_COMM_SUCCESS);
    EXPECT_EQ(fs_read_text(m_testFile), text);
    EXPECT_EQ(fs_size(m_testFile), text.size());
}

TEST_F(TestFilesystemUtilsFile, write_text_file_append_success)
{
    std::string text = "1234\n";
    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);

    EXPECT_EQ(write_text_file(m_testFile, text, false), ERR_COMM_SUCCESS);
    EXPECT_EQ(fs_read_text(m_testFile), text);
#if OS_WINDOWS
    EXPECT_EQ(fs_size(m_testFile), text.length() + 1);
#else
    EXPECT_EQ(fs_size(m_testFile), text.length());
#endif

    EXPECT_EQ(write_text_file(m_testFile, text, false), ERR_COMM_SUCCESS);
    EXPECT_EQ(fs_read_text(m_testFile), text + text);
#if OS_WINDOWS
    EXPECT_EQ(fs_size(m_testFile), 2 * (text.length() + 1));
#else
    EXPECT_EQ(fs_size(m_testFile), 2 * (text.length()));
#endif
}

TEST_F(TestFilesystemUtilsFile, write_text_file_nonexistent)
{
    EXPECT_NE(write_text_file(m_testFile, ""), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, write_text_file_target_invalid)
{
    EXPECT_NE(write_text_file(m_processDir, ""), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, copy_file_success)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";

    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(write_text_file(m_testFile, text), ERR_COMM_SUCCESS);
    EXPECT_EQ(copy_file(m_testFile, newFile), ERR_COMM_SUCCESS);

    EXPECT_EQ(fs_read_text(m_testFile), fs_read_text(newFile));
    EXPECT_EQ(fs_size(m_testFile), fs_size(newFile));

    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, copy_file_over_write_success)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(copy_file(m_testFile, newFile), ERR_COMM_SUCCESS);

    EXPECT_EQ(fs_read_text(newFile), "");
    EXPECT_EQ(fs_size(newFile), 0);

    std::string text = "12345";
    EXPECT_EQ(write_text_file(m_testFile, text), ERR_COMM_SUCCESS);
    EXPECT_EQ(copy_file(m_testFile, newFile, true), ERR_COMM_SUCCESS);
    EXPECT_EQ(fs_read_text(m_testFile), fs_read_text(newFile));
    EXPECT_EQ(fs_size(m_testFile), fs_size(newFile));
    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, copy_file_src_nonexistent)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    EXPECT_NE(copy_file(m_testFile, newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, copy_file_src_invalid)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    EXPECT_NE(copy_file(m_processDir, newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, copy_file_dst_invalid)
{
    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_NE(copy_file(m_testFile, m_processDir), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, copy_file_dst_already_exist)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(copy_file(m_testFile, newFile), ERR_COMM_SUCCESS);

    EXPECT_EQ(fs_read_text(newFile), "");
    EXPECT_EQ(fs_size(newFile), 0);
    std::string text = "1234";
    EXPECT_EQ(write_text_file(m_testFile, text), ERR_COMM_SUCCESS);
    EXPECT_NE(copy_file(m_testFile, newFile, false), ERR_COMM_SUCCESS);
    EXPECT_NE(fs_read_text(m_testFile), fs_read_text(newFile));
    EXPECT_NE(fs_size(m_testFile), fs_size(newFile));
    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, rename_file_success)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";
    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);

    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(write_text_file(m_testFile, text), ERR_COMM_SUCCESS);

    EXPECT_EQ(rename_file(m_testFile, newFile), ERR_COMM_SUCCESS);

    EXPECT_FALSE(file_exists(m_testFile));
    EXPECT_TRUE(file_exists(newFile));

    EXPECT_EQ(fs_read_text(newFile), text);
    EXPECT_EQ(fs_size(newFile), text.size());

    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, rename_file_dest_already_exist)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";
    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);

    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(create_file(newFile), ERR_COMM_SUCCESS);

    EXPECT_TRUE(file_exists(m_testFile));
    EXPECT_TRUE(file_exists(newFile));

    EXPECT_EQ(write_text_file(m_testFile, text), ERR_COMM_SUCCESS);

    EXPECT_NE(rename_file(m_testFile, newFile), ERR_COMM_SUCCESS);

    EXPECT_TRUE(file_exists(m_testFile));

    EXPECT_EQ(fs_read_text(newFile), "");
    EXPECT_EQ(fs_size(newFile), 0);

    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, rename_file_overwrite)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";
    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);

    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(create_file(newFile), ERR_COMM_SUCCESS);

    EXPECT_TRUE(file_exists(m_testFile));
    EXPECT_TRUE(file_exists(newFile));

    EXPECT_EQ(write_text_file(m_testFile, text), ERR_COMM_SUCCESS);

    EXPECT_EQ(rename_file(m_testFile, newFile, true), ERR_COMM_SUCCESS);

    EXPECT_FALSE(file_exists(m_testFile));
    EXPECT_TRUE(file_exists(newFile));

    EXPECT_EQ(fs_read_text(newFile), text);
    EXPECT_EQ(fs_size(newFile), text.size());

    EXPECT_EQ(delete_file(newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, rename_file_src_nonexistent)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";

    EXPECT_NE(rename_file(m_testFile, newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, rename_file_src_invalid)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";

    EXPECT_NE(rename_file(m_processDir, newFile), ERR_COMM_SUCCESS);
}

TEST_F(TestFilesystemUtilsFile, rename_file_dest_invalid)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";

    EXPECT_EQ(create_file(m_testFile), ERR_COMM_SUCCESS);
    EXPECT_EQ(write_text_file(m_testFile, text), ERR_COMM_SUCCESS);

    EXPECT_NE(rename_file(m_testFile, m_processDir), ERR_COMM_SUCCESS);
}

}  // namespace utils_test::filesystem_test
