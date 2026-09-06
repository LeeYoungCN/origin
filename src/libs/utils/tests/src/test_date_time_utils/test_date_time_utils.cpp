
#include <cstdint>
#include <string_view>

#include "common/constants/date_time_constants.h"
#include "common/types/date_time_types.h"
#include "gtest/gtest.h"
#include "utils/date_time_utils.h"

using namespace origin::date_time;

namespace utils_test::date_time_test {
TEST(TestDateTimeUtilsMonthName, case)
{
    for (uint32_t i = 0; i < 20; i++) {
        auto len1 = get_month_full_name(i).length();
        auto len2 = get_month_abbr_name(i).length();
        if (i >= MIN_MONTH && i <= MAX_MONTH) {
            EXPECT_GT(len1, 0);
            EXPECT_GT(len2, 0);
        } else {
            EXPECT_EQ(len1, 0);
            EXPECT_EQ(len2, 0);
        }
    }
}

TEST(TestDateTimeUtilsWeekdayName, case)
{
    for (uint32_t i = 0; i < 20; i++) {
        auto len1 = get_weekday_full_name(i).length();
        auto len2 = get_weekday_abbr_name(i).length();
        if (i >= MIN_WEEKDAY && i <= MAX_WEEKDAY) {
            EXPECT_GT(len1, 0);
            EXPECT_GT(len2, 0);
        } else {
            EXPECT_EQ(len1, 0);
            EXPECT_EQ(len2, 0);
        }
    }
}

TEST(TestTimestampAndDateTimeSt, case)
{
    for (uint32_t i = 0; i < 100; i++) {
        TimestampMs nowTime = get_now_timestamp_ms();
        DateTimeSt nowDateTime = timestamp_to_date_time(nowTime);
        EXPECT_EQ(date_time_to_timestamp(nowDateTime), nowTime);
        sleep_ms(10);
    }
}

}  // namespace utils_test::date_time_test
