#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "common/constants/filesystem_constants.h"
#include "detail/common.hpp"
#include "gtest/gtest.h"
#include "logging/log_level.hpp"
#include "logging/log_msg.hpp"
#include "logging/log_source.hpp"
#include "logging/sinks/rotating_file_sink.hpp"
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

    std::string logContent = std::string(100, 'a');
    size_t logMsgSize = logContent.size() + LF_LENGTH;

    static void InitLogFileList(std::vector<std::uint32_t>& existFileIdx, uint32_t startIdx,
                                uint32_t existFileCount);
    static void AppendLogFileList(std::vector<std::uint32_t>& existFileIdx, uint32_t appendCount);
    static void CreateLogFiles(const std::string& logFile,
                               const std::vector<std::uint32_t>& existFileIdx,
                               uint32_t sleepMs = 5);

    void InsertLogMsg(RotatingFileSink& sink, uint32_t maxFileSize, uint32_t rotateCount) const;
    void TestRotate(const testing::TestInfo* test_info, uint32_t startIdx, uint32_t existFileCount,
                    uint32_t rotateCount);
    void TestRotateAndDelete(const testing::TestInfo* test_info, uint32_t maxFiles,
                             uint32_t startIdx, uint32_t existFileCount, uint32_t rotateCount);

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

void TestRotatingFileSink::InitLogFileList(std::vector<std::uint32_t>& existFileIdx,
                                           const uint32_t startIdx, const uint32_t existFileCount)
{
    for (uint32_t i = 0; i < existFileCount; ++i) {
        uint32_t nextIdx = startIdx + i;
        if (nextIdx > RotatingFileSink::MAX_INDEX) {
            nextIdx -= RotatingFileSink::MAX_INDEX;
        }
        existFileIdx.push_back(nextIdx);
    }
}

void TestRotatingFileSink::AppendLogFileList(std::vector<std::uint32_t>& existFileIdx,
                                             const uint32_t appendCount)
{
    for (uint32_t i = 0; i < appendCount; ++i) {
        uint32_t nextIdx = (existFileIdx.empty() ? 0 : existFileIdx.back()) + 1;
        if (nextIdx > RotatingFileSink::MAX_INDEX) {
            nextIdx -= RotatingFileSink::MAX_INDEX;
        }
        existFileIdx.push_back(nextIdx);
    }
}

void TestRotatingFileSink::CreateLogFiles(const std::string& logFile,
                                          const std::vector<std::uint32_t>& existFileIdx,
                                          const uint32_t sleepMs)
{
    for (const auto& idx : existFileIdx) {
        std::string const file = logFile + "." + std::to_string(idx);
        create_file(file);
        sleep_ms(sleepMs);  // 确保文件修改时间不同
    }
}

void TestRotatingFileSink::InsertLogMsg(RotatingFileSink& sink, uint32_t maxFileSize,
                                        const uint32_t rotateCount) const
{
    sink.set_pattern("%v");

    size_t currSize = 0;
    while (currSize < rotateCount * maxFileSize) {
        LogMsg const logMsg = create_log_msg(LOG_SRC_LOCAL, "noname", LogLevel::ERR, logContent);
        sink.log(logMsg);
        currSize += logMsgSize;
    }
}

void TestRotatingFileSink::TestRotate(const testing::TestInfo* test_info, uint32_t startIdx,
                                      const uint32_t existFileCount, const uint32_t rotateCount)
{
    const std::string logFile = join_paths({_dir, get_logger_name(test_info) + ".log"});
    std::vector<uint32_t> existFileIdx;

    InitLogFileList(existFileIdx, startIdx, existFileCount);

    CreateLogFiles(logFile, existFileIdx, 10);

    constexpr uint32_t maxFileSize = 1024;
    RotatingFileSink sink(logFile, maxFileSize, RotatingFileSink::DEFAULT_MAX_FILES, false);

    InsertLogMsg(sink, maxFileSize, rotateCount);
    AppendLogFileList(existFileIdx, rotateCount);

    EXPECT_TRUE(file_exists(logFile));
    EXPECT_LT(get_file_size(logFile), maxFileSize);

    const uint32_t totalFileCount = existFileCount + rotateCount;
    ASSERT_TRUE(sink.get_file_list().size() == totalFileCount);

    for (uint32_t i = existFileCount; i < totalFileCount; i++) {
        const uint32_t nextIdx = existFileIdx[i];

        std::string const nextLogFile = logFile + "." + std::to_string(nextIdx);

        EXPECT_TRUE(file_exists(nextLogFile)) << nextLogFile;
        if (i >= existFileCount) {
            EXPECT_LT(get_file_size(nextLogFile), maxFileSize);
            EXPECT_GT(get_file_size(nextLogFile), maxFileSize - logMsgSize);
        }
    }
}

void TestRotatingFileSink::TestRotateAndDelete(const testing::TestInfo* test_info,
                                               const uint32_t maxFiles, const uint32_t startIdx,
                                               const uint32_t existFileCount,
                                               const uint32_t rotateCount)
{
    const std::string logFile = join_paths({_dir, get_logger_name(test_info) + ".log"});
    constexpr uint32_t maxFileSize = 1024;

    std::vector<uint32_t> existFileIdx;

    InitLogFileList(existFileIdx, startIdx, existFileCount);

    CreateLogFiles(logFile, existFileIdx, 10);

    RotatingFileSink sink(logFile, maxFileSize, maxFiles, false);

    InsertLogMsg(sink, maxFileSize, rotateCount);

    AppendLogFileList(existFileIdx, rotateCount);

    ASSERT_TRUE(sink.get_file_list().size() <= maxFiles);

    for (uint32_t i = 0; i < existFileIdx.size(); ++i) {
        const uint32_t idx = existFileIdx[i];
        std::string const file = logFile + "." + std::to_string(idx);
        if (i < existFileIdx.size() - maxFiles) {
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
    std::string const logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
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
    const std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});

    // valid file
    std::vector<std::uint32_t> validFileIdx;

    for (uint32_t i = RotatingFileSink::MIN_INDEX; i <= 100; ++i) {
        validFileIdx.emplace_back(i);
    }

    constexpr uint32_t startIdx = 200;
    constexpr uint32_t step = 200;
    for (uint32_t i = startIdx; i <= RotatingFileSink::MAX_INDEX; i += step) {
        validFileIdx.emplace_back(i);
    }

    CreateLogFiles(logFile, validFileIdx);

    // invalid file
    create_file(logFile);
    create_file(logFile + ".0");
    create_file(logFile + ".00");
    create_file(logFile + ".01");
    create_file(logFile + ".");
    create_file(logFile + ".a");
    create_file(logFile + "a");
    create_file(logFile + "." + std::to_string(RotatingFileSink::MAX_INDEX + 1));

    const RotatingFileSink sink(logFile, false);

    const auto fileList = sink.get_file_list();
    EXPECT_EQ(fileList.size(), validFileIdx.size());

    for (uint32_t i = 0; i < validFileIdx.size(); ++i) {
        std::string const file = logFile + "." + std::to_string(validFileIdx[i]);
        EXPECT_EQ(fileList[i], file);
    }
}

TEST_F(TestRotatingFileSink, set_and_get_max_file_size)
{
    const std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    const RotatingFileSink sink(logFile, false);

    constexpr uint32_t maxFileSize = 1024;
    sink.set_max_file_size(maxFileSize);
    EXPECT_EQ(sink.max_file_size(), maxFileSize);

    sink.set_max_file_size(0);
    EXPECT_EQ(sink.max_file_size(), maxFileSize);
}

TEST_F(TestRotatingFileSink, set_and_get_max_files)
{
    const std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    const RotatingFileSink sink(logFile, false);

    constexpr uint32_t maxFiles = 5;
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
    constexpr uint32_t startIdx = RotatingFileSink::MAX_INDEX - 3;
    constexpr uint32_t fileCount = 8;
    constexpr uint32_t rotateCount = 5;

    TestRotate(test_info_, startIdx, fileCount, rotateCount);
}

TEST_F(TestRotatingFileSink, rotate_with_existing_files_wrap_around)
{
    constexpr uint32_t startIdx = RotatingFileSink::MAX_INDEX - 1;
    constexpr uint32_t fileCount = 8;
    constexpr uint32_t rotateCount = 6;

    TestRotate(test_info_, startIdx, fileCount, rotateCount);
}

TEST_F(TestRotatingFileSink, rotate_with_existing_files_wrap_around_large)
{
    constexpr uint32_t startIdx = RotatingFileSink::MAX_INDEX - 5;
    constexpr uint32_t fileCount = 3;
    constexpr uint32_t rotateCount = 10;

    TestRotate(test_info_, startIdx, fileCount, rotateCount);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_no_existing_files)
{
    constexpr uint32_t maxFiles = 3;
    constexpr uint32_t rotateCnt = 5;

    constexpr uint32_t startIdx = RotatingFileSink::MIN_INDEX;
    constexpr uint32_t existFileCount = 0;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_existing_files)
{
    constexpr uint32_t maxFiles = 3;
    constexpr uint32_t rotateCnt = 5;

    constexpr uint32_t startIdx = RotatingFileSink::MIN_INDEX;
    constexpr uint32_t existFileCount = 3;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_existing_files_wrap_around)
{
    constexpr uint32_t maxFiles = 5;
    constexpr uint32_t rotateCnt = 10;

    constexpr uint32_t startIdx = RotatingFileSink::MAX_INDEX - 1;
    constexpr uint32_t existFileCount = 3;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_existing_files_wrap_around_large)
{
    constexpr uint32_t maxFiles = 5;
    constexpr uint32_t rotateCnt = 10;

    constexpr uint32_t startIdx = RotatingFileSink::MAX_INDEX - 3;
    constexpr uint32_t existFileCount = 3;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}

TEST_F(TestRotatingFileSink, rotate_and_delete_with_zero_max_files)
{
    constexpr uint32_t maxFiles = 0;
    constexpr uint32_t rotateCnt = 10;

    constexpr uint32_t startIdx = RotatingFileSink::MIN_INDEX;
    constexpr uint32_t existFileCount = 3;

    TestRotateAndDelete(test_info_, maxFiles, startIdx, existFileCount, rotateCnt);
}
}  // namespace logging_test
