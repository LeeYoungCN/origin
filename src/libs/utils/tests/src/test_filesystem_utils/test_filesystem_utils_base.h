/**
 * @file test_filesystem_utils_base.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-30
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "gtest/gtest.h"
#include "origin/common/common_error_code.h"
#include "origin/filesystem/filesystem.h"
#include "origin/process/process.h"
#include "origin/thread/thread.h"

namespace utils_test::filesystem_test {
using namespace origin::filesystem;
using namespace origin::process;
using namespace origin::thread;

// 测试辅助：返回旧哨兵值，失败时与兼容重载语义一致
inline std::string fs_cwd()
{
    std::string out;
    (void)get_curr_working_dir(out);
    return out;
}
inline std::string fs_read_text(std::string_view path)
{
    std::string out;
    (void)read_text_file(path, out);
    return out;
}
inline FileSize fs_size(std::string_view path)
{
    FileSize out = 0;
    (void)get_file_size(path, out);
    return out;
}
inline EntryType fs_type(std::string_view path)
{
    EntryType out = EntryType::UNKNOWN;
    (void)get_entry_type(path, out);
    return out;
}
inline std::string fs_join(const std::vector<std::string>& parts)
{
    std::string out;
    (void)join_paths(parts, out);
    return out;
}
inline std::string fs_join(std::initializer_list<std::string> parts)
{
    return fs_join(std::vector<std::string>(parts));
}
inline std::string fs_abs(std::string_view relPath, std::string_view baseDir = "")
{
    std::string out;
    (void)to_absolute_path(relPath, baseDir, out);
    return out;
}
inline std::string fs_normalize(std::string_view path)
{
    std::string out;
    (void)normalize_path(path, out);
    return out;
}

class TestFilesystemUtilsBase : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override
    {
        m_process = get_proc_path();
        EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
        m_processDir = get_directory(m_process);
        EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
        m_workDir = fs_cwd();
        EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
        m_pid = get_curr_proc_id();
        EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    };
    void TearDown() override {};

protected:
    std::string m_process;
    std::string m_processDir;
    std::string m_workDir;
    ProcessId m_pid;
};

}  // namespace utils_test::filesystem_test
