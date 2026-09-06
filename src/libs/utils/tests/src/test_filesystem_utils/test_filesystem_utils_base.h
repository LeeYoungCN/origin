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

#include "common/common_error_code.h"
#include "gtest/gtest.h"
#include "utils/filesystem_utils.h"
#include "utils/process_utils.h"
#include "utils/thread_utils.h"

namespace utils_test::filesystem_test {
using namespace origin::filesystem;
using namespace origin::process;
using namespace origin::thread;
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
        m_workDir = get_curr_working_dir();
        EXPECT_EQ(get_thread_last_err(), ERR_COMM_SUCCESS);
    };
    void TearDown() override {};

protected:
    std::string m_process;
    std::string m_processDir;
    std::string m_workDir;
};

}  // namespace utils_test::filesystem_test
