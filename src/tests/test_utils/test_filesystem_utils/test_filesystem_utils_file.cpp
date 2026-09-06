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
#include "common/constants/filesystem_constants.h"
#include "common/macros/compiler.h"
#include "gtest/gtest.h"
#include "test_filesystem_utils_base.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

namespace test::test_utils::test_filesystem_utils {

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
    m_testFile = to_absolute_path(m_testFilename, m_processDir);
}

void TestFilesystemUtilsFile::TearDown()
{
    TestFilesystemUtilsBase::TearDown();
    EXPECT_TRUE(delete_file(m_testFile));
}

TEST_F(TestFilesystemUtilsFile, file_exists)
{
    EXPECT_TRUE(file_exists(m_process));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, file_exists_nonexistent)
{
    EXPECT_FALSE(file_exists(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, file_exists_type_invalid)
{
    EXPECT_FALSE(file_exists(m_processDir));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FILE) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, create_file_already_exists)
{
    EXPECT_TRUE(create_file(m_process));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_ALREADY_EXISTS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, create_file_type_invalid)
{
    EXPECT_FALSE(create_file(m_processDir));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FILE) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, create_file_success)
{
    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, create_file_dir_nonexistent)
{
    auto newFile = to_absolute_path(m_testFilename, m_processDir + "/nonexistent");
    EXPECT_FALSE(create_file(newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, delete_file_dir_nonexistent)
{
    auto newFile = to_absolute_path(m_testFilename, m_processDir + "/nonexistent");
    EXPECT_TRUE(delete_file(newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, delete_file_type_invalid)
{
    EXPECT_FALSE(delete_file(m_processDir));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FILE) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, delete_file_file_exist)
{
    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_TRUE(delete_file(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_TRUE(delete_file(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, read_file_success)
{
    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(read_text_file(m_testFile), "");
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(get_file_size(m_testFile), 0);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, read_file_target_invalid)
{
    EXPECT_EQ(read_text_file(m_processDir), "");
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FILE) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, read_file_nonexistent)
{
    EXPECT_EQ(read_text_file(m_testFile), "");
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, write_text_file_success)
{
    std::string text = "1234";
    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_TRUE(write_text_file(m_testFile, text));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(read_text_file(m_testFile), text);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(get_file_size(m_testFile), text.size());
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, write_text_file_overwrite_success)
{
    std::string text = "1234";
    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();

    EXPECT_TRUE(write_text_file(m_testFile, text, true));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(read_text_file(m_testFile), text);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(get_file_size(m_testFile), text.size());
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();

    EXPECT_TRUE(write_text_file(m_testFile, text, true));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(read_text_file(m_testFile), text);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(get_file_size(m_testFile), text.size());
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, write_text_file_append_success)
{
    std::string text = "1234\n";
    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();

    EXPECT_TRUE(write_text_file(m_testFile, text, false));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(read_text_file(m_testFile), text);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
#if OS_WINDOWS
    EXPECT_EQ(get_file_size(m_testFile), text.length() + 1);
#else
    EXPECT_EQ(get_file_size(m_testFile), text.length());
#endif
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();

    EXPECT_TRUE(write_text_file(m_testFile, text, false));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(read_text_file(m_testFile), text + text);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
#if OS_WINDOWS
    EXPECT_EQ(get_file_size(m_testFile), 2 * (text.length() + 1));
#else
    EXPECT_EQ(get_file_size(m_testFile), 2 * (text.length()));
#endif
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, write_text_file_nonexistent)
{
    EXPECT_FALSE(write_text_file(m_testFile, ""));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND);
}

TEST_F(TestFilesystemUtilsFile, write_text_file_target_invalid)
{
    EXPECT_FALSE(write_text_file(m_processDir, ""));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FILE);
}

TEST_F(TestFilesystemUtilsFile, copy_file_success)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";

    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_TRUE(write_text_file(m_testFile, text));
    EXPECT_TRUE(copy_file(m_testFile, newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();

    EXPECT_EQ(read_text_file(m_testFile), read_text_file(newFile));
    EXPECT_EQ(get_file_size(m_testFile), get_file_size(newFile));

    EXPECT_TRUE(delete_file(newFile));
}

TEST_F(TestFilesystemUtilsFile, copy_file_over_write_success)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_TRUE(copy_file(m_testFile, newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();

    EXPECT_EQ(read_text_file(newFile), "");
    EXPECT_EQ(get_file_size(newFile), 0);

    std::string text = "12345";
    EXPECT_TRUE(write_text_file(m_testFile, text));
    EXPECT_TRUE(copy_file(m_testFile, newFile, true));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_EQ(read_text_file(m_testFile), read_text_file(newFile));
    EXPECT_EQ(get_file_size(m_testFile), get_file_size(newFile));
    EXPECT_TRUE(delete_file(newFile));
}

TEST_F(TestFilesystemUtilsFile, copy_file_src_nonexistent)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    EXPECT_FALSE(copy_file(m_testFile, newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, copy_file_src_invalid)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    EXPECT_FALSE(copy_file(m_processDir, newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FILE) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, copy_file_dst_invalid)
{
    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
    EXPECT_FALSE(copy_file(m_testFile, m_processDir));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FILE) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, copy_file_dst_already_exist)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_TRUE(copy_file(m_testFile, newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();

    EXPECT_EQ(read_text_file(newFile), "");
    EXPECT_EQ(get_file_size(newFile), 0);
    std::string text = "1234";
    EXPECT_TRUE(write_text_file(m_testFile, text));
    EXPECT_FALSE(copy_file(m_testFile, newFile, false));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_ALREADY_EXISTS) << get_thread_last_err_msg();
    EXPECT_NE(read_text_file(m_testFile), read_text_file(newFile));
    EXPECT_NE(get_file_size(m_testFile), get_file_size(newFile));
    EXPECT_TRUE(delete_file(newFile));
}

TEST_F(TestFilesystemUtilsFile, rename_file_success)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";
    EXPECT_TRUE(delete_file(newFile));

    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_TRUE(write_text_file(m_testFile, text));

    EXPECT_TRUE(rename_file(m_testFile, newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();

    EXPECT_FALSE(file_exists(m_testFile));
    EXPECT_TRUE(file_exists(newFile));

    EXPECT_EQ(read_text_file(newFile), text);
    EXPECT_EQ(get_file_size(newFile), text.size());

    EXPECT_TRUE(delete_file(newFile));
}

TEST_F(TestFilesystemUtilsFile, rename_file_dest_already_exist)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";
    EXPECT_TRUE(delete_file(newFile));

    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_TRUE(create_file(newFile));

    EXPECT_TRUE(file_exists(m_testFile));
    EXPECT_TRUE(file_exists(newFile));

    EXPECT_TRUE(write_text_file(m_testFile, text));

    EXPECT_FALSE(rename_file(m_testFile, newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_ALREADY_EXISTS) << get_thread_last_err_msg();

    EXPECT_TRUE(file_exists(m_testFile));

    EXPECT_EQ(read_text_file(newFile), "");
    EXPECT_EQ(get_file_size(newFile), 0);

    EXPECT_TRUE(delete_file(newFile));
}

TEST_F(TestFilesystemUtilsFile, rename_file_overwrite)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";
    EXPECT_TRUE(delete_file(newFile));

    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_TRUE(create_file(newFile));

    EXPECT_TRUE(file_exists(m_testFile));
    EXPECT_TRUE(file_exists(newFile));

    EXPECT_TRUE(write_text_file(m_testFile, text));

    EXPECT_TRUE(rename_file(m_testFile, newFile, true));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();

    EXPECT_FALSE(file_exists(m_testFile));
    EXPECT_TRUE(file_exists(newFile));

    EXPECT_EQ(read_text_file(newFile), text);
    EXPECT_EQ(get_file_size(newFile), text.size());

    EXPECT_TRUE(delete_file(newFile));
}

TEST_F(TestFilesystemUtilsFile, rename_file_src_nonexistent)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";

    EXPECT_FALSE(rename_file(m_testFile, newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FOUND) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, rename_file_src_invalid)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";

    EXPECT_FALSE(rename_file(m_processDir, newFile));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FILE) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsFile, rename_file_dest_invalid)
{
    std::string newFile = m_processDir + PATH_SEP + "file2";
    std::string text = "12345";

    EXPECT_TRUE(create_file(m_testFile));
    EXPECT_TRUE(write_text_file(m_testFile, text));

    EXPECT_FALSE(rename_file(m_testFile, m_processDir));
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_NOT_FILE) << get_thread_last_err_msg();
}

}  // namespace test::test_utils::test_filesystem_utils
