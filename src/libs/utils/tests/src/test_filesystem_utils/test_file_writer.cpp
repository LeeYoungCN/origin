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

class TestFileWriter : public ::testing::Test {
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
    std::shared_ptr<FileWriter> m_fileWriter;
};

void TestFileWriter::SetUp()
{
    m_directory =
        join_paths({get_proc_directory(), "test_file_writer_" + std::to_string(get_curr_proc_id())});
    m_testFile = join_paths({m_directory, m_filename + m_suffix});
}

void TestFileWriter::TearDown()
{
    if (m_fileWriter) {
        m_fileWriter->close();
    }
    delete_dir(m_directory);
}

TEST_F(TestFileWriter, file_name_empty)
{
    EXPECT_THROW(FileWriter(""), std::invalid_argument);
}

TEST_F(TestFileWriter, file_name_process)
{
    m_fileWriter = std::make_shared<FileWriter>(m_testFile);
    EXPECT_EQ(m_fileWriter->filename(), m_filename + m_suffix);
    EXPECT_EQ(m_fileWriter->filename_stem(), m_filename);
    EXPECT_EQ(m_fileWriter->directory(), m_directory);
}

TEST_F(TestFileWriter, open_success)
{
    m_fileWriter = std::make_shared<FileWriter>(m_testFile);
    m_fileWriter->open(true);
    EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_TRUE(file_exists(m_testFile));
}

TEST_F(TestFileWriter, append_size)
{
    m_fileWriter = std::make_shared<FileWriter>(m_testFile);
    m_fileWriter->open(true);
    EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->size(), 0);
    EXPECT_TRUE(file_exists(m_testFile));

    FileSize expectSize = 0;
    for (uint32_t i = 0; i < 1024; i++) {
        std::string line = "line" + std::to_string(i);
        expectSize += line.length();
        m_fileWriter->write(line);
        EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->size(), expectSize);
        EXPECT_GE(m_fileWriter->size(), get_file_size(m_testFile));
    }
    m_fileWriter->flush();
    m_fileWriter->close();
    EXPECT_EQ(m_fileWriter->size(), get_file_size(m_testFile));
}

TEST_F(TestFileWriter, append_line_size)
{
    m_fileWriter = std::make_shared<FileWriter>(m_testFile);
    m_fileWriter->open(true);
    EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->size(), 0);
    EXPECT_TRUE(file_exists(m_testFile));

    FileSize expectSize = 0;
    for (uint32_t i = 0; i < 1024; i++) {
        std::string line = "line" + std::to_string(i);
        expectSize += line.length() + origin::filesystem::LF_LENGTH;
        m_fileWriter->write_line(line);
        EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->size(), expectSize);
        EXPECT_GE(m_fileWriter->size(), get_file_size(m_testFile));
    }
    m_fileWriter->flush();
    EXPECT_EQ(m_fileWriter->size(), get_file_size(m_testFile));
    m_fileWriter->close();
    EXPECT_EQ(m_fileWriter->size(), get_file_size(m_testFile));
}

TEST_F(TestFileWriter, append_mode)
{
    m_fileWriter = std::make_shared<FileWriter>(m_testFile);
    m_fileWriter->open(true);
    EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->size(), 0);
    EXPECT_TRUE(file_exists(m_testFile));

    FileSize expectSize = 0;
    for (uint32_t i = 0; i < 64; i++) {
        std::string line = "line" + std::to_string(i);
        expectSize += line.length() + origin::filesystem::LF_LENGTH;
        m_fileWriter->write_line(line);
        EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
        m_fileWriter->close();
        m_fileWriter->reopen(false);
        EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->size(), expectSize);
        EXPECT_GE(m_fileWriter->size(), get_file_size(m_testFile));
    }
    m_fileWriter->flush();
    EXPECT_EQ(m_fileWriter->size(), get_file_size(m_testFile));
    m_fileWriter->close();
    EXPECT_EQ(m_fileWriter->size(), get_file_size(m_testFile));
}

}  // namespace utils_test::test_file_writer
