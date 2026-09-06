/**
 * @file filesystem_test.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-29
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <cstddef>

#include "common/constants/filesystem_constants.h"
#include "common/types/filesystem_types.h"
#include "gtest/gtest.h"
#include "test_filesystem_utils_base.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

namespace utils_test::filesystem_test {

using namespace origin::filesystem;

class TestFilesystemUtilsPath : public TestFilesystemUtilsBase {};

TEST_F(TestFilesystemUtilsPath, ProcessPath)
{
    EXPECT_FALSE(m_process.empty());
    EXPECT_EQ(get_entry_type(m_process), EntryType::FILE);
}

TEST_F(TestFilesystemUtilsPath, ProcessDir)
{
    EXPECT_FALSE(m_processDir.empty());
    EXPECT_EQ(get_entry_type(m_processDir), EntryType::DIRECTORY);
}

TEST_F(TestFilesystemUtilsPath, WorkingDirectory)
{
    EXPECT_FALSE(m_workDir.empty());
    EXPECT_EQ(get_entry_type(m_workDir), EntryType::DIRECTORY);
}

TEST_F(TestFilesystemUtilsPath, AbsolutePath)
{
    EXPECT_TRUE(is_absolute_path(m_process));
    EXPECT_TRUE(is_absolute_path(m_workDir));
    EXPECT_TRUE(is_absolute_path(m_processDir));
}

TEST_F(TestFilesystemUtilsPath, ToAbsolute)
{
    auto baseName = get_filename(m_process);
    std::string relPah = std::string(".") + PATH_SEP + baseName;
    EXPECT_FALSE(is_absolute_path(relPah));
    auto absFile = to_absolute_path(relPah, m_processDir);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_TRUE(is_absolute_path(absFile));
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_EQ(get_entry_type(m_process), EntryType::FILE);
}

TEST_F(TestFilesystemUtilsPath, BaseName)
{
    auto filename = get_filename_stem(m_process);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    auto extention = get_extension(m_process);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    std::string fileWithExt = filename + extention;
    auto baseName = get_filename(m_process);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_EQ(baseName, fileWithExt);
}

TEST_F(TestFilesystemUtilsPath, normalize_path)
{
    auto baseName = get_filename(m_process);
    auto testPath = m_processDir + PATH_SEP + "." + PATH_SEP + baseName;
    auto rstPath = normalize_path(testPath);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_process, rstPath);
    EXPECT_TRUE(is_absolute_path(rstPath));
}

TEST_F(TestFilesystemUtilsPath, NormalizePath_WithDots)
{
    std::string test = "a/b/../c/./d";
    std::string expect(MAX_PATH_STD, '\0');
    auto len = snprintf(expect.data(), MAX_PATH_STD, "a%sc%sd", PATH_SEP, PATH_SEP);
    expect.resize(static_cast<size_t>(len));
    auto result = normalize_path(test);
    EXPECT_EQ(result, expect);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsPath, NormalizePath_EmptyPath)
{
    std::string result = normalize_path("");

    EXPECT_EQ(result, "");
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsPath, JoinPaths_empty)
{
    EXPECT_EQ(join_paths({}), "");
    EXPECT_EQ(get_thread_last_err(), ERR_UTILS_PATH_INVALID) << get_thread_last_err_msg();
}

TEST_F(TestFilesystemUtilsPath, JoinPaths_Success)
{
    std::vector<std::string> test = {"a", "b", "c"};
    std::string expect(MAX_PATH_STD, '\0');
    auto size = snprintf(expect.data(), MAX_PATH_STD, "a%sb%sc", PATH_SEP, PATH_SEP);
    expect.resize(static_cast<size_t>(size));
    EXPECT_EQ(join_paths(test), expect);
    EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS) << get_thread_last_err_msg();
}

}  // namespace utils_test::filesystem_test
