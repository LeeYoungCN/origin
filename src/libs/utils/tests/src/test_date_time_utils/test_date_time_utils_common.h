
#ifndef TEST_UTILS_TEST_DATE_TIME_UTILS_COMMON_H
#define TEST_UTILS_TEST_DATE_TIME_UTILS_COMMON_H

#include <cstdint>
#include <ctime>
#include <string_view>

#include "common/types/date_time_types.h"

namespace utils_test::date_time_test {

std::tm TimestampToTimeInfo(TimestampMs ts);

void TestTimeBuffer(const std::string_view& format, uint32_t length, TimestampMs timestamp);

void TestTimeBuffer(const std::string_view& format, uint32_t length, std::tm timeInfo,
                    const DateTimeSt& dateTime);

void TestTimeString(const std::string_view& format, TimestampMs timestamp);

void TestTimeString(const std::string_view& format, std::tm timeInfo, const DateTimeSt& dateTime);
}  // namespace utils_test::date_time_test
#endif  // TEST_UTILS_TEST_DATE_TIME_UTILS_COMMON_H
