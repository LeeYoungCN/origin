/**
 * @file date_time_constants.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-27
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once
#ifndef COMMON_CONSTANTS_DATE_TIME_CONSTANTS_H
#define COMMON_CONSTANTS_DATE_TIME_CONSTANTS_H

#include <cstdint>

#include "origin/common/types/date_time_types.h"

namespace origin::date_time {

[[maybe_unused]] constexpr int32_t DATE_TIME_START_YEAR = 1900;
[[maybe_unused]] constexpr int32_t DATE_TIME_START_MONTH = 1;

// 1970-01-01 00:00:00 UTC对应的时间戳（毫秒）
[[maybe_unused]] constexpr TimestampMs UNIX_EPOCH_MS = 0;

// 纪元年份（Unix时间戳起始年份）
// EPOCH（通常译为 “纪元” 或 “时间起点”）指的是时间戳计数的起始点，是用于统一时间计算的基准时间。
[[maybe_unused]] constexpr int32_t EPOCH_YEAR = 1970;
[[maybe_unused]] constexpr int32_t EPOCH_MONTH = 1;   // 1月（注意：部分API使用0-based月份）
[[maybe_unused]] constexpr int32_t EPOCH_DAY = 1;     // 1日
[[maybe_unused]] constexpr int32_t EPOCH_HOUR = 0;    // 0时
[[maybe_unused]] constexpr int32_t EPOCH_MINUTE = 0;  // 0分
[[maybe_unused]] constexpr int32_t EPOCH_SECOND = 0;  // 0秒

// Windows FILETIME 与 Unix 时间戳的转换常量
// FILETIME从1601-01-01开始，与Unix纪元的差值（毫秒）
// 计算方式: (1970-1601)年的总毫秒数 = 369年 * 365天 + 89个闰年天数
// 369*365=134,685 + 89=134,774天 * 86,400秒/天 = 11,644,473,600秒
// 转换为毫秒: 11,644,473,600 * 1000 = 11,644,473,600,000
[[maybe_unused]] constexpr int64_t WINDOWS_EPOCH_TO_UNIX_EPOCH_MS = 11644473600000ULL;
[[maybe_unused]] constexpr int64_t HUNDRED_NANOSECONDS_PER_MILLISECOND = 10000;

// 增加纪元时间戳常量，便于直接使用
[[maybe_unused]] constexpr int64_t UNIX_EPOCH_TIMESTAMP_SEC = 0;  // 纪元时间的秒级时间戳
[[maybe_unused]] constexpr int64_t UNIX_EPOCH_TIMESTAMP_MS = 0;   // 纪元时间的毫秒级时间戳
[[maybe_unused]] constexpr int64_t WINDOWS_EPOCH_TIMESTAMP_100NS =
    0;  // Windows纪元的100纳秒级时间戳

[[maybe_unused]] constexpr const char* DEFAULT_TIME_FMT = "%Y-%m-%d %H:%M:%S";
[[maybe_unused]] constexpr const char* MILLIS_PLACEHOLDER = "%3f";
[[maybe_unused]] constexpr const char* DEFAULT_TIME_FMT_MS = "%Y-%m-%d %H:%M:%S.%3f";
[[maybe_unused]] constexpr uint32_t MAX_TIME_STR_LEN = 256;

[[maybe_unused]] constexpr int64_t MICROS_PER_MILLI = 1000;
[[maybe_unused]] constexpr int64_t MILLIS_PER_SEC = 1000;
[[maybe_unused]] constexpr int64_t SEC_PER_MINUTE = 60;
[[maybe_unused]] constexpr int64_t MINUTE_PER_HOUR = 60;
[[maybe_unused]] constexpr int64_t HOUR_PER_DAY = 24;

[[maybe_unused]] constexpr int64_t MICROS_PER_SEC = MILLIS_PER_SEC * MICROS_PER_MILLI;

[[maybe_unused]] constexpr int64_t MILLIS_PER_MINUTE = MILLIS_PER_SEC * SEC_PER_MINUTE;
[[maybe_unused]] constexpr int64_t MILLIS_PER_HOUR = MILLIS_PER_MINUTE * MINUTE_PER_HOUR;
[[maybe_unused]] constexpr int64_t MILLIS_PER_DAY = MILLIS_PER_HOUR * HOUR_PER_DAY;

[[maybe_unused]] constexpr uint32_t MONTHS_PER_YEAR = 12;
[[maybe_unused]] constexpr uint32_t MIN_MONTH = 1;
[[maybe_unused]] constexpr uint32_t MAX_MONTH = 12;

[[maybe_unused]] constexpr uint32_t DAYS_PER_WEEK = 7;
[[maybe_unused]] constexpr uint32_t MIN_WEEKDAY = 0;
[[maybe_unused]] constexpr uint32_t MAX_WEEKDAY = 6;

[[maybe_unused]] constexpr uint32_t MIN_MINUTE = 0;
[[maybe_unused]] constexpr uint32_t MAX_MINUTE = 59;

[[maybe_unused]] constexpr uint32_t MIN_HOUR = 0;
[[maybe_unused]] constexpr uint32_t MAX_HOUR = 23;

}  // namespace origin::date_time

#endif  // COMMON_CONSTANTS_DATE_TIME_CONSTANTS_H
