#include <cstdint>
#include <ctime>
#include <string_view>
#include <tuple>
#include <vector>

#include "common/constants/date_time_constants.h"
#include "common/types/date_time_types.h"
#include "gtest/gtest.h"
#include "test_date_time_utils_common.h"
#include "utils/date_time_utils.h"

namespace {
const std::vector<std::string_view> MONTH_TEST_FORMAT = {"Y-%y-%m-%d-%H:%M %B",
                                                         "Y-%y-%m-%d-%H:%M %b",
                                                         "Y-%y-%m-%d-%H:%M %h",
                                                         "Y-%y-%m-%d-%H:%M %B %b %h"};

const std::vector<std::string_view> WEEKDAY_TEST_FORMAT = {
    "Y-%y-%m-%d-%H:%M %A", "Y-%y-%m-%d-%H:%M %a", "Y-%y-%m-%d-%H:%M %A %a"};
}  // namespace

namespace utils_test::date_time_test {

using TestBufferParam = std::tuple<std::string_view, uint32_t, uint32_t, uint32_t>;
using TestStringParam = std::tuple<std::string_view, uint32_t, uint32_t>;

using namespace origin::date_time;

class TestDateTimeUtilsFmtTimeCompBuffer : public ::testing::TestWithParam<TestBufferParam> {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};
};

TEST_P(TestDateTimeUtilsFmtTimeCompBuffer, DateTimeSt)
{
    auto format = std::get<0>(GetParam());
    auto length = std::get<1>(GetParam());
    auto month = std::get<2>(GetParam());
    auto wday = std::get<3>(GetParam());

    auto ts = get_now_timestamp_ms();
    auto dateTime = local_date_time(ts);
    auto timeInfo = TimestampToTimeInfo(ts);
    dateTime.month = month;
    dateTime.wday = wday;
    timeInfo.tm_mon = static_cast<int32_t>(month - DATE_TIME_START_MONTH);
    timeInfo.tm_wday = static_cast<int32_t>(wday);

    TestTimeBuffer(format, length, timeInfo, dateTime);
}

INSTANTIATE_TEST_SUITE_P(
    LENGTH, TestDateTimeUtilsFmtTimeCompBuffer,
    testing::Combine(testing::Values("%Y-%y-%m-%d-%H:%M:%S-%A-%a-%B-%b-%h", "111"),
                     testing::Range(1U, 50U, 5U), testing::Values(2), testing::Values(2)));

#if defined(__linux__) || defined(__APPLE__)
INSTANTIATE_TEST_SUITE_P(LINUX_AND_APPLE_MONTH, TestDateTimeUtilsFmtTimeCompBuffer,
                         testing::Combine(testing::ValuesIn(MONTH_TEST_FORMAT),
                                          testing::Values(MAX_TIME_STR_LEN),
                                          testing::Range(0U, 15U), testing::Values(2)));

INSTANTIATE_TEST_SUITE_P(LINUX_AND_APPLE_WEEKDAY, TestDateTimeUtilsFmtTimeCompBuffer,
                         testing::Combine(testing::ValuesIn(WEEKDAY_TEST_FORMAT),
                                          testing::Values(MAX_TIME_STR_LEN), testing::Values(10U),
                                          testing::Range(0U, 10U)));
#endif

#if defined(__linux__) || defined(__APPLE__)
class TestDateTimeUtilsTimeCompStr : public ::testing::TestWithParam<TestStringParam> {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};
};

TEST_P(TestDateTimeUtilsTimeCompStr, DateTimeSt)
{
    auto format = std::get<0>(GetParam());
    auto month = std::get<1>(GetParam());
    auto wday = std::get<2>(GetParam());

    auto ts = get_now_timestamp_ms();
    auto dateTime = local_date_time(ts);
    auto timeInfo = TimestampToTimeInfo(ts);
    dateTime.month = month;
    dateTime.wday = wday;
    timeInfo.tm_mon = static_cast<int32_t>(month - DATE_TIME_START_MONTH);
    timeInfo.tm_wday = static_cast<int32_t>(wday);

    TestTimeString(format, timeInfo, dateTime);
}

INSTANTIATE_TEST_SUITE_P(LINUX_AND_APPLE_MONTH, TestDateTimeUtilsTimeCompStr,
                         testing::Combine(testing::ValuesIn(MONTH_TEST_FORMAT),
                                          testing::Range(0U, 15U), testing::Values(2)));

INSTANTIATE_TEST_SUITE_P(LINUX_AND_APPLE_WEEKDAY, TestDateTimeUtilsTimeCompStr,
                         testing::Combine(testing::ValuesIn(WEEKDAY_TEST_FORMAT),
                                          testing::Values(10U), testing::Range(0U, 10U)));
#endif
}  // namespace utils_test::date_time_test
