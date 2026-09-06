#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "common/constants/filesystem_constants.h"
#include "detail/common.h"
#include "gtest/gtest.h"
#include "internal/log_msg.h"
#include "logging/log_level.h"
#include "logging/log_source.h"
#include "logging/sinks/rotating_file_sink.h"
#include "utils/date_time_utils.h"
#include "utils/filesystem_utils.h"

using namespace origin::logging;
using namespace origin::filesystem;
using namespace origin::date_time;

namespace logging_test {
class TestRotatingFileSink : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;

protected:
    std::string logContent = std::string(100, 'a');
    size_t logMsgSize = logContent.size() + LF_LENGTH;

protected:
    void InsertLogMsg(RotatingFileSink& sink, uint32_t maxFileSize, uint32_t rotateCount);
    void InitLogFileList(std::vector<std::uint32_t>& existFileIdxs, uint32_t startIdx,
                         uint32_t existFileCount);
    void AppendLogFileList(std::vector<std::uint32_t>& existFileIdxs, uint32_t appendCount);
    void CreateLogFiles(const std::string& logFile, const std::vector<std::uint32_t>& existFileIdxs,
                        uint32_t sleepMs = 5);

    void TestRotate(testing::TestInfo* test_info, uint32_t startIdx, uint32_t existFileCount,
                    uint32_t rotateCount);
    void TestRotateAndDelete(testing::TestInfo* test_info, uint32_t maxFiles, uint32_t startIdx,
                             uint32_t existFileCount, uint32_t rotateCount);

protected:
    std::string _dir = get_log_dir();
};

void TestRotatingFileSink::SetUp()
{
    create_dir(_dir);
}

void TestRotatingFileSink::TearDown()
{
    delete_dir(_dir);
}

void TestRotatingFileSink::InitLogFileList(std::vector<std::uint32_t>& existFileIdxs,
                                           uint32_t startIdx, uint32_t existFileCount)
{
    for (uint32_t i = 0; i < existFileCount; ++i) {
        uint32_t nextIdx = startIdx + i;
        if (nextIdx > RotatingFileSink::MAX_INDEX) {
            nextIdx -= RotatingFileSink::MAX_INDEX;
        }
        existFileIdxs.push_back(nextIdx);
    }
}

void TestRotatingFileSink::AppendLogFileList(std::vector<std::uint32_t>& existFileIdxs,
                                             uint32_t appendCount)
{
    for (uint32_t i = 0; i < appendCount; ++i) {
        uint32_t nextIdx = (existFileIdxs.empty() ? 0 : existFileIdxs.back()) + 1;
        if (nextIdx > RotatingFileSink::MAX_INDEX) {
            nextIdx -= RotatingFileSink::MAX_INDEX;
        }
        existFileIdxs.push_back(nextIdx);
    }
}

void TestRotatingFileSink::CreateLogFiles(const std::string& logFile,
                                          const std::vector<std::uint32_t>& existFileIdxs,
                                          uint32_t sleepMs)
{
    for (const auto& idx : existFileIdxs) {
        std::string file = logFile + "." + std::to_string(idx);
        create_file(file);
        sleep_ms(sleepMs);  // 确保文件修改时间不同
    }
}

void TestRotatingFileSink::InsertLogMsg(RotatingFileSink& sink, uint32_t maxFileSize,
                                        uint32_t rotateCount)
{
    sink.set_pattern("%v");

    size_t currSize = 0;
    while (currSize < rotateCount * maxFileSize) {
        LogMsg logMsg(LOG_SRC_LOCAL, "noname", LogLevel::ERR, logContent);
        sink.log(logMsg);
        currSize += logMsgSize;
    }
}

void TestRotatingFileSink::TestRotate(testing::TestInfo* test_info, uint32_t startIdx,
                                      uint32_t existFileCount, uint32_t rotateCount)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info) + ".log"});
    std::vector<uint32_t> existFileIdxs;

    InitLogFileList(existFileIdxs, startIdx, existFileCount);

    CreateLogFiles(logFile, existFileIdxs, 10);

    const uint32_t maxFileSize = 1024;
    RotatingFileSink sink(logFile, maxFileSize, RotatingFileSink::DEFAULT_MAX_FILES, false);

    InsertLogMsg(sink, maxFileSize, rotateCount);
    AppendLogFileList(existFileIdxs, rotateCount);

    EXPECT_TRUE(file_exists(logFile));
    EXPECT_LT(get_file_size(logFile), maxFileSize);

    uint32_t totalFileCount = existFileCount + rotateCount;
    ASSERT_TRUE(sink.get_file_list().size() == totalFileCount);

    for (uint32_t i = existFileCount; i < totalFileCount; i++) {
        uint32_t nextIdx = existFileIdxs[i];

        std::string nextLogFile = logFile + "." + std::to_string(nextIdx);

        EXPECT_TRUE(file_exists(nextLogFile)) << nextLogFile;
        if (i >= existFileCount) {
            EXPECT_LT(get_file_size(nextLogFile), maxFileSize);
            EXPECT_GT(get_file_size(nextLogFile), maxFileSize - logMsgSize);
        }
    }
}

void TestRotatingFileSink::TestRotateAndDelete(testing::TestInfo* test_info, uint32_t maxFiles,
                                               uint32_t startIdx, uint32_t existFileCount,
                                               uint32_t rotateCount)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info) + ".log"});
    const uint32_t maxFileSize = 1024;

    std::vector<uint32_t> existFileIdxs;

    InitLogFileList(existFileIdxs, startIdx, existFileCount);

    CreateLogFiles(logFile, existFileIdxs, 10);

    RotatingFileSink sink(logFile, maxFileSize, maxFiles, false);

    InsertLogMsg(sink, maxFileSize, rotateCount);

    AppendLogFileList(existFileIdxs, rotateCount);

    ASSERT_TRUE(sink.get_file_list().size() <= maxFiles);

    for (uint32_t i = 0; i < existFileIdxs.size(); ++i) {
        uint32_t idx = existFileIdxs[i];
        std::string file = logFile + "." + std::to_string(idx);
        if (i < existFileIdxs.size() - maxFiles) {
            EXPECT_FALSE(file_exists(file)) << file;
        } else {
            EXPECT_TRUE(file_exists(file)) << file;
            EXPECT_LT(get_file_size(file), maxFileSize);
            EXPECT_GT(get_file_size(file), maxFileSize - logMsgSize);
        }
    }
}

TEST_F(TestRotatingFileSink, invalid_param)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    EXPECT_THROW(RotatingFileSink(""), std::invalid_argument);
    EXPECT_THROW(RotatingFileSink(logFile, 0, RotatingFileSink::DEFAULT_MAX_FILES),
                 std::invalid_argument);
    EXPECT_THROW(
        RotatingFileSink(
            logFile, RotatingFileSink::DEFAULT_MAX_FILE_SIZE, RotatingFileSink::MAX_INDEX + 1),
        std::out_of_range);
}

TEST_F(TestRotatingFileSink, init_with_existing_files)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});

    // valid file
    std::vector<std::uint32_t> validFileIdxs;

    for (uint32_t i = RotatingFileSink::MIN_INDEX; i <= 100; ++i) {
        validFileIdxs.emplace_back(i);
    }

    const uint32_t startIdx = 200;
    const uint32_t step = 200;
    for (uint32_t i = startIdx; i <= RotatingFileSink::MAX_INDEX; i += step) {
        validFileIdxs.emplace_back(i);
    }

    CreateLogFiles(logFile, validFileIdxs);

    // invalid file
    create_file(logFile);
    create_file(logFile + ".0");
    create_file(logFile + ".00");
    create_file(logFile + ".01");
    create_file(logFile + ".");
    create_file(logFile + ".a");
    create_file(logFile + "a");
    create_file(logFile + "." + std::to_string(RotatingFileSink::MAX_INDEX + 1));

    RotatingFileSink sink(logFile, false);

    auto fileList = sink.get_file_list();
    EXPECT_EQ(fileList.size(), validFileIdxs.size());

    for (uint32_t i = 0; i < validFileIdxs.size(); ++i) {
        std::string file = logFile + "." + std::to_string(validFileIdxs[i]);
        EXPECT_EQ(fileList[i], file);
    }
}

TEST_F(TestRotatingFileSink, set_and_get_max_file_size)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    RotatingFileSink sink(logFile, false);

    const uint32_t maxFileSize = 1024;
    sink.set_max_file_size(maxFileSize);
    EXPECT_EQ(sink.max_file_size(), maxFileSize);

    sink.set_max_file_size(0);
    EXPECT_EQ(sink.max_file_size(), maxFileSize);
}

TEST_F(TestRotatingFileSink, set_and_get_max_files)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    RotatingFileSink sink(logFile, false);

    const uint32_t maxFiles = 5;
    sink.set_max_files(maxFiles);
    EXPECT_EQ(sink.max_files(), maxFiles);

    sink.set_max_files(RotatingFileSink::MAX_FILES + 1);
    EXPECT_EQ(sink.max_files(), maxFiles);
}

TEST_F(TestRotatingFileSink, rotate_with_no_existing_files)
{
    TestRotate(test_info_, 0, 0, 3);
}

TEST_F(TestRotatingFileSink, rotate_with_existing_files)
{
    const uint32_t startIdx = RotatingFileSink::MAX_INDEX - 3;
    const uint32_t fileCount = 8;
    const uint32_t rotateCount = 5;

    TestRotate(test_info_, startIdx, fileCount, rotateCount);
}

TEST_F(TestRotatingFileSink, rotate_with_existing_files_wrap_around)
{
    const uint32_t startIdx = RotatingFileSink::MAX_INDEX - 1;
    const uint32_t fileCount = 8;
    const uint32_t rotateCount = 6;

    TestRotate(test_info_, startIdx, fileCount, rotateCount);
}

TEST_F(TestRotatingFileSink, rotate_with_existing_files_wrap_around_large)
{
    const uint32_t startIdx = RotatingFileSink::MAX_INDEX - 5;
    const uint32_t fileCount = 3;
    const uint32_t rotateCount = 10;

    TestRotate(test_info_, startIdx, fileCount, rotateCount);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_no_existing_files)
{
    const uint32_t maxFiles = 3;
    const uint32_t rotateCnt = 5;

    const uint32_t startIdx = RotatingFileSink::MIN_INDEX;
    const uint32_t existFileCount = 0;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_existing_files)
{
    const uint32_t maxFiles = 3;
    const uint32_t rotateCnt = 5;

    const uint32_t startIdx = RotatingFileSink::MIN_INDEX;
    const uint32_t existFileCount = 3;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_existing_files_wrap_around)
{
    const uint32_t maxFiles = 5;
    const uint32_t rotateCnt = 10;

    const uint32_t startIdx = RotatingFileSink::MAX_INDEX - 1;
    const uint32_t existFileCount = 3;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_existing_files_wrap_around_large)
{
    const uint32_t maxFiles = 5;
    const uint32_t rotateCnt = 10;

    const uint32_t startIdx = RotatingFileSink::MAX_INDEX - 3;
    const uint32_t existFileCount = 3;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_zero_max_files)
{
    const uint32_t maxFiles = 0;
    const uint32_t rotateCnt = 10;

    const uint32_t startIdx = RotatingFileSink::MIN_INDEX;
    const uint32_t existFileCount = 3;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}
}  // namespace logging_test
