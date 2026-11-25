#include "gtest/gtest.h"

#include <cstdint>
#include <memory>
#include <string>

#include "origin/common/common_error_code.h"
#include "origin/common/constants/filesystem_constants.h"
#include "origin/common/types/filesystem_types.h"

#include "origin/filesystem/file_writer.h"
#include "origin/filesystem/filesystem.h"
#include "origin/process/process.h"
#include "origin/utils/utils_error_code.h"

#include "test_filesystem_utils_base.h"

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
    std::string procDir;
    (void)get_proc_directory(procDir);
    m_directory = utils_test::filesystem_test::fs_join(
        {procDir, "test_file_writer_" + std::to_string(get_curr_proc_id())});
    m_testFile = utils_test::filesystem_test::fs_join({m_directory, m_filename + m_suffix});
}

void TestFileWriter::TearDown()
{
    if (m_fileWriter) {
        m_fileWriter->close();
    }
    (void)delete_dir(m_directory);
}

TEST_F(TestFileWriter, file_name_empty)
{
    FileWriter writer("");
    EXPECT_EQ(writer.open(true), ERR_COMM_PARAM_EMPTY);
    EXPECT_EQ(writer.get_last_error(), ERR_COMM_PARAM_EMPTY);
    EXPECT_TRUE(writer.full_path().empty());
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
    EXPECT_EQ(m_fileWriter->open(true), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_TRUE(file_exists(m_testFile));
}

TEST_F(TestFileWriter, append_size)
{
    m_fileWriter = std::make_shared<FileWriter>(m_testFile);
    EXPECT_EQ(m_fileWriter->open(true), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->size(), 0);
    EXPECT_TRUE(file_exists(m_testFile));

    FileSize expectSize = 0;
    for (uint32_t i = 0; i < 1024; i++) {
        std::string line = "line" + std::to_string(i);
        expectSize += line.length();
        EXPECT_EQ(m_fileWriter->write(line), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->size(), expectSize);
        EXPECT_GE(m_fileWriter->size(), utils_test::filesystem_test::fs_size(m_testFile));
    }
    EXPECT_EQ(m_fileWriter->flush(), ERR_COMM_SUCCESS);
    m_fileWriter->close();
    EXPECT_EQ(m_fileWriter->size(), utils_test::filesystem_test::fs_size(m_testFile));
}

TEST_F(TestFileWriter, append_line_size)
{
    m_fileWriter = std::make_shared<FileWriter>(m_testFile);
    EXPECT_EQ(m_fileWriter->open(true), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->size(), 0);
    EXPECT_TRUE(file_exists(m_testFile));

    FileSize expectSize = 0;
    for (uint32_t i = 0; i < 1024; i++) {
        std::string line = "line" + std::to_string(i);
        expectSize += line.length() + origin::filesystem::LF_LENGTH;
        EXPECT_EQ(m_fileWriter->write_line(line), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->size(), expectSize);
        EXPECT_GE(m_fileWriter->size(), utils_test::filesystem_test::fs_size(m_testFile));
    }
    EXPECT_EQ(m_fileWriter->flush(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->size(), utils_test::filesystem_test::fs_size(m_testFile));
    m_fileWriter->close();
    EXPECT_EQ(m_fileWriter->size(), utils_test::filesystem_test::fs_size(m_testFile));
}

TEST_F(TestFileWriter, append_mode)
{
    m_fileWriter = std::make_shared<FileWriter>(m_testFile);
    EXPECT_EQ(m_fileWriter->open(true), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->size(), 0);
    EXPECT_TRUE(file_exists(m_testFile));

    FileSize expectSize = 0;
    for (uint32_t i = 0; i < 64; i++) {
        std::string line = "line" + std::to_string(i);
        expectSize += line.length() + origin::filesystem::LF_LENGTH;
        EXPECT_EQ(m_fileWriter->write_line(line), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
        m_fileWriter->close();
        EXPECT_EQ(m_fileWriter->reopen(false), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->get_last_error(), ERR_COMM_SUCCESS);
        EXPECT_EQ(m_fileWriter->size(), expectSize);
        EXPECT_GE(m_fileWriter->size(), utils_test::filesystem_test::fs_size(m_testFile));
    }
    EXPECT_EQ(m_fileWriter->flush(), ERR_COMM_SUCCESS);
    EXPECT_EQ(m_fileWriter->size(), utils_test::filesystem_test::fs_size(m_testFile));
    m_fileWriter->close();
    EXPECT_EQ(m_fileWriter->size(), utils_test::filesystem_test::fs_size(m_testFile));
}

TEST_F(TestFileWriter, write_after_close_reports_not_open)
{
    m_fileWriter = std::make_shared<FileWriter>(m_testFile);
    ASSERT_EQ(m_fileWriter->open(true), ERR_COMM_SUCCESS);
    m_fileWriter->close();

    EXPECT_EQ(m_fileWriter->write("should fail"), ERR_UTILS_FILE_NOT_OPEN);
    EXPECT_EQ(m_fileWriter->flush(), ERR_UTILS_FILE_NOT_OPEN);
    EXPECT_EQ(m_fileWriter->get_last_error(), ERR_UTILS_FILE_NOT_OPEN);
}

}  // namespace utils_test::test_file_writer
