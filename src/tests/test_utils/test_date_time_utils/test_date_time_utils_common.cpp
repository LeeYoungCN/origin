#include "test_date_time_utils_common.h"

#include "common/macros/compiler.h"

#if OS_WINDOWS
#include <windows.h>
#endif
#include <cstdint>
#include <ctime>
#include <string_view>

#include "common/constants/date_time_constants.h"
#include "common/types/date_time_types.h"
#include "gtest/gtest.h"
#include "utils/date_time_utils.h"

namespace test::test_utils::test_date_time_utils {

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
    auto dateTime = local_date_time(timestamp);
    auto timeInfo = TimestampToTimeInfo(timestamp);
    TestTimeBuffer(format, length, timeInfo, dateTime);
}

void TestTimeBuffer(const std::string_view& format, uint32_t length, std::tm timeInfo,
                    const DateTimeSt& dateTime)
{
    char bufferUtils[MAX_TIME_STR_LEN] = {'\0'};
    auto sizeUtils = format_time_buffer(bufferUtils, length, dateTime, format);

    char bufferStd[MAX_TIME_STR_LEN] = {'\0'};
    auto sizeStd = std::strftime(bufferStd, length, format.data(), &timeInfo);

    EXPECT_EQ(sizeStd, sizeUtils);

    if (sizeUtils > 0) {
        CompareString(dateTime, bufferStd, bufferUtils);
    }
}

void TestTimeString(const std::string_view& format, TimestampMs timestamp)
{
    auto dateTime = local_date_time(timestamp);
    auto timeInfo = TimestampToTimeInfo(timestamp);
    TestTimeString(format, timeInfo, dateTime);
}

void TestTimeString(const std::string_view& format, std::tm timeInfo, const DateTimeSt& dateTime)
{
    auto utilsStr = format_time_string(dateTime, format);
    std::stringstream ss;
    ss << std::put_time(&timeInfo, format.data());

    EXPECT_EQ(ss.str().size(), utilsStr.size());

    if (utilsStr.size() > 0) {
        CompareString(dateTime, ss.str(), utilsStr);
    }
}

}  // namespace test::test_utils::test_date_time_utils
