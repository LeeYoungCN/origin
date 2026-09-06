#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "common/common_error_code.h"
#include "common/constants/filesystem_constants.h"
#include "gtest/gtest.h"
#include "utils/file_writer.h"
#include "utils/filesystem_utils.h"
#include "utils/process_utils.h"

namespace utils_test::test_file_writer {
using namespace origin::filesystem;
using namespace origin::process;

class TestFileAppender : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;

protected:
    std::string m_filename{"test_file"};
    std::string m_suffix{".log"};
    std::string m_directory;
    std::string m_testFile;
    std::shared_ptr<FileWriter> m_appender;
};

void TestFileAppender::SetUp()
{
    m_directory = join_paths({get_proc_directory(), "file_writer"});
    m_testFile = join_paths({m_directory, m_filename + m_suffix});
}

void TestFileAppender::TearDown()
{
    if (m_appender) {
        m_appender->close();
    }
    delete_dir(m_directory);
}

TEST_F(TestFileAppender, file_name_empty)
{
    EXPECT_THROW(FileWriter(""), std::invalid_argument);
}

TEST_F(TestFileAppender, file_name_process)
{
    m_appender = std::make_shared<FileWriter>(m_testFile);
    EXPECT_EQ(m_appender->filename(), m_filename + m_suffix);
    EXPECT_EQ(m_appender->filename_stem(), m_filename);
    EXPECT_EQ(m_appender->directory(), m_directory);
}

TEST_F(TestFileAppender, open_success)
{
    m_appender = std::make_shared<FileWriter>(m_testFile);
    m_appender->open(true);
    EXPECT_EQ(m_appender->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_TRUE(file_exists(m_testFile));
}

TEST_F(TestFileAppender, append_size)
{
    m_appender = std::make_shared<FileWriter>(m_testFile);
    m_appender->open(true);
    EXPECT_EQ(m_appender->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_appender->size(), 0);
    EXPECT_TRUE(file_exists(m_testFile));

    FileSize expectSize = 0;
    for (uint32_t i = 0; i < 1024; i++) {
        std::string line = "line" + std::to_string(i);
        expectSize += line.length();
        m_appender->write(line);
        EXPECT_EQ(m_appender->get_last_error(), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_appender->size(), expectSize);
        EXPECT_GE(m_appender->size(), get_file_size(m_testFile));
    }
    m_appender->flush();
    m_appender->close();
    EXPECT_EQ(m_appender->size(), get_file_size(m_testFile));
}

TEST_F(TestFileAppender, append_line_size)
{
    m_appender = std::make_shared<FileWriter>(m_testFile);
    m_appender->open(true);
    EXPECT_EQ(m_appender->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_appender->size(), 0);
    EXPECT_TRUE(file_exists(m_testFile));

    FileSize expectSize = 0;
    for (uint32_t i = 0; i < 1024; i++) {
        std::string line = "line" + std::to_string(i);
        expectSize += line.length() + origin::filesystem::LF_LENGTH;
        m_appender->write_line(line);
        EXPECT_EQ(m_appender->get_last_error(), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_appender->size(), expectSize);
        EXPECT_GE(m_appender->size(), get_file_size(m_testFile));
    }
    m_appender->flush();
    EXPECT_EQ(m_appender->size(), get_file_size(m_testFile));
    m_appender->close();
    EXPECT_EQ(m_appender->size(), get_file_size(m_testFile));
}

TEST_F(TestFileAppender, append_mode)
{
    m_appender = std::make_shared<FileWriter>(m_testFile);
    m_appender->open(true);
    EXPECT_EQ(m_appender->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_appender->size(), 0);
    EXPECT_TRUE(file_exists(m_testFile));

    FileSize expectSize = 0;
    for (uint32_t i = 0; i < 64; i++) {
        std::string line = "line" + std::to_string(i);
        expectSize += line.length() + origin::filesystem::LF_LENGTH;
        m_appender->write_line(line);
        EXPECT_EQ(m_appender->get_last_error(), ERR_COMM_SUCCESS);
        m_appender->close();
        m_appender->reopen(false);
        EXPECT_EQ(m_appender->get_last_error(), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_appender->size(), expectSize);
        EXPECT_GE(m_appender->size(), get_file_size(m_testFile));
    }
    m_appender->flush();
    EXPECT_EQ(m_appender->size(), get_file_size(m_testFile));
    m_appender->close();
    EXPECT_EQ(m_appender->size(), get_file_size(m_testFile));
}

}  // namespace utils_test::test_file_writer
