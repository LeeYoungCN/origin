
#include <cstdint>
#include <string_view>

#include "origin/common/common_error_code.h"
#include "origin/common/constants/date_time_constants.h"
#include "origin/common/types/date_time_types.h"
#include "origin/common/types/error_code_types.h"

#include "origin/date_time/date_time.h"
#include "origin/utils/utils_error_code.h"

#include "gtest/gtest.h"

using namespace origin::date_time;

namespace utils_test::date_time_test {
TEST(TestDateTimeUtilsMonthName, case)
{
    for (uint32_t i = 0; i < 20; i++) {
        std::string_view full{};
        std::string_view abbr{};
        const ErrorCode rcFull = get_month_full_name(i, full);
        const ErrorCode rcAbbr = get_month_abbr_name(i, abbr);
        if (i >= MIN_MONTH && i <= MAX_MONTH) {
            EXPECT_EQ(rcFull, ERR_COMM_SUCCESS);
            EXPECT_EQ(rcAbbr, ERR_COMM_SUCCESS);
            EXPECT_GT(full.length(), 0);
            EXPECT_GT(abbr.length(), 0);
        } else {
            EXPECT_EQ(rcFull, ERR_UTILS_MONTH_INVALID);
            EXPECT_EQ(rcAbbr, ERR_UTILS_MONTH_INVALID);
            EXPECT_EQ(full.length(), 0);
            EXPECT_EQ(abbr.length(), 0);
        }
    }
}

TEST(TestDateTimeUtilsWeekdayName, case)
{
    for (uint32_t i = 0; i < 20; i++) {
        std::string_view full{};
        std::string_view abbr{};
        const ErrorCode rcFull = get_weekday_full_name(i, full);
        const ErrorCode rcAbbr = get_weekday_abbr_name(i, abbr);
        if (i >= MIN_WEEKDAY && i <= MAX_WEEKDAY) {
            EXPECT_EQ(rcFull, ERR_COMM_SUCCESS);
            EXPECT_EQ(rcAbbr, ERR_COMM_SUCCESS);
            EXPECT_GT(full.length(), 0);
            EXPECT_GT(abbr.length(), 0);
        } else {
            EXPECT_EQ(rcFull, ERR_UTILS_WEEKDAY_INVALID);
            EXPECT_EQ(rcAbbr, ERR_UTILS_WEEKDAY_INVALID);
            EXPECT_EQ(full.length(), 0);
            EXPECT_EQ(abbr.length(), 0);
        }
    }
}

TEST(TestTimestampAndDateTimeSt, case)
{
    for (uint32_t i = 0; i < 100; i++) {
        TimestampMs nowTime = get_now_timestamp_ms();
        DateTimeSt nowDateTime{};
        ASSERT_EQ(timestamp_to_date_time(nowTime, TimeZone::LOCAL, nowDateTime), ERR_COMM_SUCCESS);
        TimestampMs backTime = 0;
        ASSERT_EQ(date_time_to_timestamp(nowDateTime, backTime), ERR_COMM_SUCCESS);
        EXPECT_EQ(backTime, nowTime);
        sleep_ms(10);
    }
}

}  // namespace utils_test::date_time_test
