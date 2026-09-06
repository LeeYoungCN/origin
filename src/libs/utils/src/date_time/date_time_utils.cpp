#include "utils/date_time_utils.h"

#include "common/macros/compiler.h"

#if OS_WINDOWS
#include <windows.h>

#include "common/constants/date_time_constants.h"

#else
#include <cerrno>
#include <chrono>
#include <thread>
#endif  // OS_WINDOWS

#include <cstdint>
#include <ctime>

#include "common/constants/date_time_constants.h"
#include "common/debug/debug_logger.h"
#include "common/types/date_time_types.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

namespace {
using namespace origin::thread;
using namespace origin::date_time;
#if OS_WINDOWS
#endif  // OS_WINDOWS

bool safe_localtime(time_t timer, tm& timeInfo)
{
#if OS_WINDOWS
    // Windows 使用 localtime_s
    auto err = localtime_s(&timeInfo, &timer);
    if (err != 0) {
        set_thread_last_err(ERR_UTILS_TIMESTAMP_INVALID);
        // 特别处理负数时间戳的错误提示
        if (timer < 0) {
            ORIGIN_DEBUG_WARN(
                "localtime_s may not support negative. time: {}, err: {}", timer, err);
        } else {
            ORIGIN_DEBUG_ERR("localtime_s time failed. {}, err: {}", timer, err);
        }
        return false;
    }
#else
    // Linux/macOS 使用 localtime_r
    if (localtime_r(&timer, &timeInfo) == nullptr) {
        set_thread_last_err(ERR_UTILS_TIMESTAMP_INVALID);
        ORIGIN_DEBUG_ERR("localtime_r time failed. {}, errno: {}", timer, errno);
        return false;
    }
#endif
    set_thread_last_err(ERR_COMM_SUCCESS);
    return true;
}

bool safe_gmtime(time_t timer, tm& timeInfo)
{
#if OS_WINDOWS
    // Windows下使用gmtime_s，增加负数时间戳检查
    errno_t err = gmtime_s(&timeInfo, &timer);
    if (err != 0) {
        set_thread_last_err(ERR_UTILS_TIMESTAMP_INVALID);
        // 针对负数时间戳的错误做特殊提示
        if (timer < 0) {
            ORIGIN_DEBUG_WARN("gmtime_s may not support negative time: {}, err: {}", timer, err);
        } else {
            ORIGIN_DEBUG_ERR("gmtime_s time failed. {}, err: {}", timer, err);
        }
        return false;
    }
#else
    // Linux/macOS使用gmtime_r（对负数时间戳支持更完善）
    if (gmtime_r(&timer, &timeInfo) == nullptr) {
        set_thread_last_err(ERR_UTILS_TIMESTAMP_INVALID);
        ORIGIN_DEBUG_ERR("Gmtime_r time failed. {}, errno: {}.", timer, errno);
        return false;
    }
#endif
    set_thread_last_err(ERR_COMM_SUCCESS);
    return true;
}

void std_tm_2_date_time_st(const std::tm timeInfo, int32_t millis, DateTimeSt& dateTime)
{
    dateTime.year = static_cast<uint32_t>(DATE_TIME_START_YEAR + timeInfo.tm_year);
    dateTime.month = static_cast<uint32_t>(DATE_TIME_START_MONTH + timeInfo.tm_mon);
    dateTime.day = static_cast<uint32_t>(timeInfo.tm_mday);
    dateTime.hour = static_cast<uint32_t>(timeInfo.tm_hour);
    dateTime.minute = static_cast<uint32_t>(timeInfo.tm_min);
    dateTime.second = static_cast<uint32_t>(timeInfo.tm_sec);
    dateTime.millis = static_cast<uint32_t>(millis);
    dateTime.wday = static_cast<uint32_t>(timeInfo.tm_wday);
    dateTime.yday = static_cast<uint32_t>(timeInfo.tm_yday);
}

void date_time_st_to_std_tm(const DateTimeSt& dateTime, std::tm& timeInfo)
{
    timeInfo.tm_year = static_cast<int32_t>(dateTime.year - DATE_TIME_START_YEAR);
    timeInfo.tm_mon = static_cast<int32_t>(dateTime.month - DATE_TIME_START_MONTH);
    timeInfo.tm_mday = static_cast<int32_t>(dateTime.day);
    timeInfo.tm_hour = static_cast<int32_t>(dateTime.hour);
    timeInfo.tm_min = static_cast<int32_t>(dateTime.minute);
    timeInfo.tm_sec = static_cast<int32_t>(dateTime.second);
    timeInfo.tm_wday = static_cast<int32_t>(dateTime.wday);
    timeInfo.tm_yday = static_cast<int32_t>(dateTime.yday);
}

}  // namespace

namespace origin::date_time {

TimestampMs get_now_timestamp_ms()
{
    set_thread_last_err(ERR_COMM_SUCCESS);
#if OS_WINDOWS
    FILETIME ft;
    // 获取当前系统时间，以FILETIME格式存储（从Windows纪元1601-01-01 00:00:00开始的100纳秒间隔数）
    GetSystemTimeAsFileTime(&ft);

    // 将FILETIME的高低位 DWORD 合并为64位无符号整数，得到完整的100纳秒单位时间戳
    constexpr int FILETIME_HIGH_SHIFT_BITS = 32;  // FILETIME高32位左移位数
    int64_t file_time =
        (static_cast<int64_t>(ft.dwHighDateTime) << FILETIME_HIGH_SHIFT_BITS) | ft.dwLowDateTime;

    // 转换为Unix时间戳（毫秒级）：
    // 1. 先将100纳秒单位转换为毫秒（除以10000，因1毫秒=10000×100纳秒）
    // 2. 减去Windows纪元到Unix纪元（1970-01-01 00:00:00）的毫秒差值，得到标准Unix时间戳
    return file_time / HUNDRED_NANOSECONDS_PER_MILLISECOND  // 转换为毫秒
           - WINDOWS_EPOCH_TO_UNIX_EPOCH_MS;                // 校正到Unix纪元

#else
    std::chrono::time_point now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<TimestampMs>(ms.count());
#endif
}

DateTimeSt get_now_date_time()
{
    return timestamp_to_date_time(get_now_timestamp_ms());
}

DateTimeSt local_date_time(TimestampMs timestamp)
{
    return timestamp_to_date_time(timestamp, TimeZone::LOCAL);
}

DateTimeSt utc_date_time(TimestampMs timestamp)
{
    return timestamp_to_date_time(timestamp, TimeZone::UTC);
}

DateTimeSt timestamp_to_date_time(TimestampMs timestamp, TimeZone timeZone)
{
    auto timer = static_cast<std::time_t>(timestamp / MILLIS_PER_SEC);
    auto millis = static_cast<int32_t>(timestamp % MILLIS_PER_SEC);

    std::tm timeInfo{};
    DateTimeSt dateTime{};
    bool rst = false;
    switch (timeZone) {
        case TimeZone::UTC:
            rst = safe_gmtime(timer, timeInfo);
            break;
        case TimeZone::LOCAL:
        default:
            rst = safe_localtime(timer, timeInfo);
    }

    if (!rst) {
        ORIGIN_DEBUG_ERR("Get time info failed. zone: {}, msg: {}.",
                         GetTimeZoneString(timeZone),
                         get_thread_last_err_msg());
    } else {
        std_tm_2_date_time_st(timeInfo, millis, dateTime);
        set_thread_last_err(ERR_COMM_SUCCESS);
    }
    return dateTime;
}

TimestampMs date_time_to_timestamp(const DateTimeSt& dateTime)
{
    std::tm tm{};
    date_time_st_to_std_tm(dateTime, tm);
    std::time_t secStamp = std::mktime(&tm);
    if (secStamp == -1) {
        return -1;
    }
    return static_cast<TimestampMs>(secStamp) * MILLIS_PER_SEC + dateTime.millis;
}

DurationMs diff_timestamp(TimestampMs x, TimestampMs y)
{
    return x - y;
}

DurationMs diff_date_time(const DateTimeSt& x, const DateTimeSt& y)
{
    return date_time_to_timestamp(x) - date_time_to_timestamp(y);
}

void sleep_ms(DurationMs ms)
{
#if OS_WINDOWS
    ::Sleep(static_cast<DWORD>(ms));
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

}  // namespace origin::date_time
