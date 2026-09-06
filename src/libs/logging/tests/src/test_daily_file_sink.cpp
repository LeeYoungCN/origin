#include <sys/types.h>

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "common/constants/date_time_constants.h"
#include "common/types/date_time_types.h"
#include "detail/common.h"
#include "gtest/gtest.h"
#include "internal/log_msg.h"
#include "logging/log_level.h"
#include "logging/log_source.h"
#include "logging/sinks/daily_file_sink.h"
#include "utils/date_time_utils.h"
#include "utils/filesystem_utils.h"

namespace logging_test {
using namespace origin::date_time;
using namespace origin::logging;
using namespace origin::filesystem;

class TestDailyFileSink : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;

protected:
    std::string calc_log_file(std::string_view baseFile, TimestampMs time);
    void TestRotate(const testing::TestInfo* testInfo, uint32_t hour, uint32_t minute,
                    uint32_t rotationDays, uint32_t dayInterVal = 1);
    void TestRotateAndDelete(const testing::TestInfo* testInfo, uint32_t maxFiles,
                             uint32_t existFiles, uint32_t rotationDays);

protected:
    std::string _dir = get_log_dir();
};

void TestDailyFileSink::SetUp()
{
    delete_dir(_dir);
    create_dir(_dir);
}

void TestDailyFileSink::TearDown()
{
    delete_dir(_dir);
}

std::string TestDailyFileSink::calc_log_file(std::string_view baseFile, TimestampMs time)
{
    std::string filenameStem = get_filename_stem(baseFile);
    std::string extention = get_extension(baseFile);
    std::string filename =
        std::format("{}.{}{}", filenameStem, format_time_string(time, "%Y%m%d"), extention);
    return join_paths({_dir, filename});
}

void TestDailyFileSink::TestRotate(const testing::TestInfo* testInfo, uint32_t hour,
                                   uint32_t minute, uint32_t rotationDays, uint32_t dayInterval)
{
    TimestampMs now = get_now_timestamp_ms();
    DateTimeSt nowDateTime = timestamp_to_date_time(now);
    DateTimeSt rotationDateTime = nowDateTime;
    rotationDateTime.hour = hour;
    rotationDateTime.minute = minute;
    bool isAfterNow(diff_date_time(rotationDateTime, nowDateTime) > 0);

    std::string baseFile = join_paths({_dir, get_logger_name(testInfo) + ".log"});
    DailyFileSink sink(baseFile, hour, minute);
    sink.set_pattern("[%d][%l][%n]: %v");
    sink.set_level(LogLevel::TRACE);

    LogMsg logMsg(LOG_SRC_LOCAL, testInfo->name(), LogLevel::ERR, "");
    std::vector<std::string> expectFiles;

    for (uint32_t i = 0; i <= rotationDays; ++i) {
        logMsg.timestamp = now + i * dayInterval * MILLIS_PER_DAY;
        logMsg.data = std::to_string(i);
        sink.log(logMsg);
        if (i != rotationDays) {
            expectFiles.push_back(
                calc_log_file(baseFile, logMsg.timestamp - (isAfterNow ? MILLIS_PER_DAY : 0)));
        }
    }

    auto files = sink.get_file_list();
    EXPECT_EQ(files.size(), rotationDays);
    EXPECT_EQ(files.size(), expectFiles.size());
    for (uint32_t i = 0; i < rotationDays; ++i) {
        EXPECT_EQ(files[i], expectFiles[i]);
        EXPECT_TRUE(file_exists(files[i]));
    }
}

void TestDailyFileSink::TestRotateAndDelete(const testing::TestInfo* testInfo, uint32_t maxFiles,
                                            uint32_t existFiles, uint32_t rotationDays)
{
    TimestampMs now = get_now_timestamp_ms();
    std::string baseFile = join_paths({_dir, get_logger_name(testInfo) + ".log"});

    std::vector<std::string> allFilesList;
    for (uint32_t i = 0; i < existFiles; i++) {
        auto timeStamp = now - (i + 1) * MILLIS_PER_DAY;
        std::string file = calc_log_file(baseFile, timeStamp);
        create_file(file);
        allFilesList.push_back(file);
        sleep_ms(10);
    }

    DailyFileSink sink(baseFile, 0, 0, maxFiles);
    sink.set_pattern("[%d][%l][%n]: %v");
    sink.set_level(LogLevel::TRACE);

    LogMsg logMsg(LOG_SRC_LOCAL, testInfo->name(), LogLevel::ERR, "");

    for (uint32_t i = 0; i <= rotationDays; ++i) {
        logMsg.timestamp = now + i * MILLIS_PER_DAY;
        logMsg.data = std::to_string(i);
        sink.log(logMsg);
        if (i != rotationDays) {
            allFilesList.push_back(calc_log_file(baseFile, logMsg.timestamp));
        }
    }

    auto currFiles = sink.get_file_list();
    EXPECT_LE(currFiles.size(), maxFiles);

    for (uint32_t i = 0, j = 0; i < allFilesList.size(); ++i) {
        const auto& file = allFilesList[i];
        if (i < allFilesList.size() - maxFiles) {
            EXPECT_FALSE(file_exists(file));
        } else {
            EXPECT_EQ(currFiles[j++], file);
            EXPECT_TRUE(file_exists(file));
        }
    }
}

TEST_F(TestDailyFileSink, create_normal)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    DailyFileSink sink(logFile);
    EXPECT_TRUE(file_exists(sink.file()));
    EXPECT_EQ(sink.file(), logFile);
}

TEST_F(TestDailyFileSink, create_when_param_invalid)
{
    std::string baseFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    EXPECT_THROW(DailyFileSink(""), std::invalid_argument);

    EXPECT_THROW(DailyFileSink(baseFile, MAX_HOUR + 1, 0, DailyFileSink::DEFAULT_MAX_FILES),
                 std::out_of_range);

    EXPECT_THROW(DailyFileSink(baseFile, 0, MAX_MINUTE + 1, DailyFileSink::DEFAULT_MAX_FILES),
                 std::out_of_range);
    EXPECT_THROW(DailyFileSink(baseFile, 0, MAX_MINUTE + 1, DailyFileSink::MAX_FILES + 1),
                 std::out_of_range);
}

TEST_F(TestDailyFileSink, init_with_existing_files)
{
    std::string filenameStem = get_logger_name(test_info_);
    std::string baseFile = join_paths({_dir, filenameStem + ".log"});
    TimestampMs now = get_now_timestamp_ms();
    std::vector<std::string> validFiles;
    for (int32_t i = 10; i >= 0; --i) {
        auto timeStamp = now - i * MILLIS_PER_DAY;
        std::string file = calc_log_file(baseFile, timeStamp);
        create_file(file);
        validFiles.push_back(file);
        sleep_ms(10);
    }

    create_file(baseFile);
    create_file(baseFile + ".log");
    create_file(join_paths({_dir, filenameStem + ".2026.log"}));
    create_file(join_paths({_dir, filenameStem + ".202601.log"}));
    create_file(join_paths({_dir, filenameStem + ".20260732.log"}));
    create_file(join_paths({_dir, filenameStem + ".00000000.log"}));
    create_file(join_paths({_dir, filenameStem + ".test.log"}));

    DailyFileSink sink(baseFile);
    auto fileList = sink.get_file_list();
    ASSERT_EQ(fileList.size(), validFiles.size());
    for (uint32_t i = 0; i < validFiles.size(); ++i) {
        EXPECT_EQ(validFiles[i], fileList[i]);
    }
}

TEST_F(TestDailyFileSink, rotate_normal)
{
    const uint32_t rotationHour = 0;
    const uint32_t rotationMinute = 0;
    const uint32_t days = 10;

    TestRotate(test_info_, rotationHour, rotationMinute, days);
}

TEST_F(TestDailyFileSink, rotate_day_interval)
{
    const uint32_t rotationHour = 0;
    const uint32_t rotationMinute = 0;
    const uint32_t days = 10;
    const uint32_t interval = 2;

    TestRotate(test_info_, rotationHour, rotationMinute, days, interval);
}

TEST_F(TestDailyFileSink, rotate_when_rotating_after_now)
{
    const uint32_t rotationHour = MAX_HOUR;
    const uint32_t rotationMinute = MAX_MINUTE;
    const uint32_t days = 10;

    TestRotate(test_info_, rotationHour, rotationMinute, days);
}

TEST_F(TestDailyFileSink, rotate_when_existing_files)
{
    const uint32_t rotationHour = MAX_HOUR;
    const uint32_t rotationMinute = MAX_MINUTE;
    const uint32_t days = 10;

    TestRotate(test_info_, rotationHour, rotationMinute, days);
}

TEST_F(TestDailyFileSink, delete_when_no_existing_files)
{
    const uint32_t maxFiles = 5;
    const uint32_t existFiles = 0;
    const uint32_t rotationDays = 10;

    TestRotateAndDelete(test_info_, maxFiles, existFiles, rotationDays);
}

TEST_F(TestDailyFileSink, delete_when_existing_files)
{
    const uint32_t maxFiles = 5;
    const uint32_t existFiles = 5;
    const uint32_t rotationDays = 10;

    TestRotateAndDelete(test_info_, maxFiles, existFiles, rotationDays);
}
}  // namespace logging_test
