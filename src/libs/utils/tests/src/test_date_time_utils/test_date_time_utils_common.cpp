#include "test_date_time_utils_common.h"

#include "origin/common/common_error_code.h"
#include "origin/common/macros/compiler.h"
#include "origin/common/types/error_code_types.h"

#if OS_WINDOWS
// NOLINTNEXTLINE(misc-include-cleaner): SDK 窄头文件的平台宏由 windows.h 建立
#include <windows.h>

#include <winnls.h>
#endif
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

#include "gtest/gtest.h"
#include "origin/common/constants/date_time_constants.h"
#include "origin/common/types/date_time_types.h"
#include "origin/date_time/date_time.h"

namespace utils_test::date_time_test {

using namespace origin::date_time;

tm TimestampToTimeInfo(TimestampMs ts)
{
    auto timer = static_cast<std::time_t>(ts / MILLIS_PER_SEC);
    std::tm timeInfo{};

#if OS_WINDOWS
    localtime_s(&timeInfo, &timer);
#else
    localtime_r(&timer, &timeInfo);
#endif
    return timeInfo;
}

void CompareString(const DateTimeSt& dateTime, std::string stdTimeStr,
                   const std::string& utilsTimeStr)
{
    char buffer[4];  // 3位数字 + 终止符
    snprintf(buffer, sizeof(buffer), "%03d", dateTime.millis);
    std::string three_digit = buffer;
    std::string placeHolder(MILLIS_PLACEHOLDER);
    size_t pos = 0;
    // 替换所有%3f占位符
    while ((pos = stdTimeStr.find(placeHolder, pos)) != std::string::npos) {
        stdTimeStr.replace(pos, placeHolder.length(), three_digit);
        pos += three_digit.length();  // 跳过已替换部分，避免重复替换
    }
    EXPECT_EQ(stdTimeStr, utilsTimeStr);
}

void TestTimeBuffer(const std::string_view& format, uint32_t length, TimestampMs timestamp)
{
    DateTimeSt dateTime{};
    ASSERT_EQ(local_date_time(timestamp, dateTime), ERR_COMM_SUCCESS);
    auto timeInfo = TimestampToTimeInfo(timestamp);
    TestTimeBuffer(format, length, timeInfo, dateTime);
}

void TestTimeBuffer(const std::string_view& format, uint32_t length, std::tm timeInfo,
                    const DateTimeSt& dateTime)
{
    char bufferUtils[MAX_TIME_STR_LEN] = {'\0'};
    size_t sizeUtils = 0;
    const ErrorCode rc = format_time_buffer(bufferUtils, length, dateTime, format, sizeUtils);
    if (sizeUtils > 0) {
        ASSERT_EQ(rc, ERR_COMM_SUCCESS);
    } else {
        EXPECT_NE(rc, ERR_COMM_SUCCESS);
    }

    char bufferStd[MAX_TIME_STR_LEN] = {'\0'};
    auto sizeStd = std::strftime(bufferStd, length, format.data(), &timeInfo);

    EXPECT_EQ(sizeStd, sizeUtils);

    if (sizeUtils > 0) {
        CompareString(dateTime, bufferStd, bufferUtils);
    }
}

void TestTimeString(const std::string_view& format, TimestampMs timestamp)
{
    DateTimeSt dateTime{};
    ASSERT_EQ(local_date_time(timestamp, dateTime), ERR_COMM_SUCCESS);
    auto timeInfo = TimestampToTimeInfo(timestamp);
    TestTimeString(format, timeInfo, dateTime);
}

void TestTimeString(const std::string_view& format, std::tm timeInfo, const DateTimeSt& dateTime)
{
    std::string utilsStr;
    ASSERT_EQ(format_time_string(dateTime, format, utilsStr), ERR_COMM_SUCCESS);
    std::stringstream ss;
    ss << std::put_time(&timeInfo, format.data());

    EXPECT_EQ(ss.str().size(), utilsStr.size());

    if (!utilsStr.empty()) {
        CompareString(dateTime, ss.str(), utilsStr);
    }
}

}  // namespace utils_test::date_time_test
