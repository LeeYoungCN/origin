/**
 * @file date_time_utils_string.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-29
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

#include "common/constants/date_time_constants.h"
#include "common/debug/debug_logger.h"
#include "utils/date_time_utils.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

namespace {
// 星期英文名称（全程）
// 索引0-6对应：星期日-星期六
const std::array<std::string_view, 7> WEEKDAY_FULL_NAMES = {
    "Sunday",     // 星期日
    "Monday",     // 星期一
    "Tuesday",    // 星期二
    "Wednesday",  // 星期三
    "Thursday",   // 星期四
    "Friday",     // 星期五
    "Saturday"    // 星期六
};

// 星期英文名称（简称）
// 索引0-6对应：星期日-星期六
const std::array<std::string_view, 7> WEEKDAY_ABBR_NAMES = {
    "Sun",  // 星期日
    "Mon",  // 星期一
    "Tue",  // 星期二
    "Wed",  // 星期三
    "Thu",  // 星期四
    "Fri",  // 星期五
    "Sat"   // 星期六
};

// 月份英文名称（全程）
// 索引0-11对应：一月-十二月
const std::array<std::string_view, 12> MONTH_FULL_NAMES = {
    "January",    // 一月
    "February",   // 二月
    "March",      // 三月
    "April",      // 四月
    "May",        // 五月
    "June",       // 六月
    "July",       // 七月
    "August",     // 八月
    "September",  // 九月
    "October",    // 十月
    "November",   // 十一月
    "December"    // 十二月
};

// 月份英文名称（简称）
// 索引0-11对应：一月-十二月
const std::array<std::string_view, 12> MONTH_ABBR_NAMES = {
    "Jan",  // 一月
    "Feb",  // 二月
    "Mar",  // 三月
    "Apr",  // 四月
    "May",  // 五月
    "Jun",  // 六月
    "Jul",  // 七月
    "Aug",  // 八月
    "Sep",  // 九月
    "Oct",  // 十月
    "Nov",  // 十一月
    "Dec"   // 十二月
};

}  // namespace

namespace origin::date_time {
using namespace origin::thread;

std::string_view get_month_full_name(uint32_t month)
{
    if (month < MIN_MONTH || month > MAX_MONTH) {
        set_thread_last_err(ERR_UTILS_MONTH_INVALID);
        ORIGIN_DEBUG_ERR(
            "Month invalid out of range [{}, {}]. weekday: {}.", MIN_MONTH, MAX_MONTH, month);
        return "";
    }
    set_thread_last_err(ERR_COMM_SUCCESS);
    return MONTH_FULL_NAMES.at(month - MIN_MONTH);
}

std::string_view get_month_abbr_name(uint32_t month)
{
    if (month < MIN_MONTH || month > MAX_MONTH) {
        set_thread_last_err(ERR_UTILS_MONTH_INVALID);
        ORIGIN_DEBUG_ERR(
            "Month invalid out of range [{}, {}]. weekday: {}.", MIN_MONTH, MAX_MONTH, month);
        return "";
    }
    set_thread_last_err(ERR_COMM_SUCCESS);
    return MONTH_ABBR_NAMES.at(month - MIN_MONTH);
}

std::string_view get_weekday_full_name(uint32_t weekday)
{
    if (weekday < MIN_WEEKDAY || weekday > MAX_WEEKDAY) {
        set_thread_last_err(ERR_UTILS_WEEKDAY_INVALID);
        ORIGIN_DEBUG_ERR("Weekday invalid out of range [{}, {}]. weekday: {}.",
                         MIN_WEEKDAY,
                         MAX_WEEKDAY,
                         weekday);
        return "";
    }
    set_thread_last_err(ERR_COMM_SUCCESS);
    return WEEKDAY_FULL_NAMES.at(weekday);
}

std::string_view get_weekday_abbr_name(uint32_t weekday)
{
    if (weekday < MIN_WEEKDAY || weekday > MAX_WEEKDAY) {
        set_thread_last_err(ERR_UTILS_WEEKDAY_INVALID);
        ORIGIN_DEBUG_ERR("Weekday invalid out of range [{}, {}]. weekday: {}.",
                         MIN_WEEKDAY,
                         MAX_WEEKDAY,
                         weekday);
        return "";
    }
    set_thread_last_err(ERR_COMM_SUCCESS);
    return WEEKDAY_ABBR_NAMES.at(weekday);
}

std::string format_time_string(TimestampMs timestamp, const std::string_view& format,
                               TimeZone timeZone)
{
    std::string timeString;
    append_time_string(timeString, timestamp, format, timeZone);
    return timeString;
}

std::string format_time_string(const DateTimeSt& dateTime, const std::string_view& format)
{
    std::string timeString;
    append_time_string(timeString, dateTime, format);
    return timeString;
}

void append_time_string(std::string& timeString, TimestampMs timestamp,
                        const std::string_view& format, TimeZone timeZone)
{
    auto dateTime = timestamp_to_date_time(timestamp, timeZone);
    append_time_string(timeString, dateTime, format);
}

void append_time_string(std::string& timeString, const DateTimeSt& dateTime,
                        const std::string_view& format)
{
    size_t formatIdx = 0;

    auto insertString = [&](const std::string_view& name) -> void {
        std::string_view insertName = name.empty() ? "?" : name;
        std::format_to(std::back_inserter(timeString), "{}", insertName);
    };

    for (; formatIdx < format.length(); formatIdx++) {
        if (format[formatIdx] != '%') {
            timeString.append(std::string_view(format.data() + formatIdx, 1));
            continue;
        }
        formatIdx++;
        if (formatIdx >= format.length()) {
            timeString.append("%");
            break;
        }
        bool success = true;
        switch (format[formatIdx]) {
            case 'Y':
                std::format_to(std::back_inserter(timeString), "{:04}", dateTime.year);
                break;
            case 'y':
                std::format_to(std::back_inserter(timeString), "{:02}", dateTime.year % 100);
                break;
            case 'm':
                std::format_to(std::back_inserter(timeString), "{:02}", dateTime.month);
                break;
            case 'd':
                std::format_to(std::back_inserter(timeString), "{:02}", dateTime.day);
                break;
            case 'H':
                std::format_to(std::back_inserter(timeString), "{:02}", dateTime.hour);
                break;
            case 'M':
                std::format_to(std::back_inserter(timeString), "{:02}", dateTime.minute);
                break;
            case 'S':
                std::format_to(std::back_inserter(timeString), "{:02}", dateTime.second);
                break;
            case 'B':  // 完整月份名称
                insertString(get_month_full_name(dateTime.month));
                break;
            case 'b':  // 缩写月份名称
            case 'h':  // 缩写月份名称
                insertString(get_month_abbr_name(dateTime.month));
                break;
            case 'A':  // 完整星期名称
                insertString(get_weekday_full_name(dateTime.wday));
                break;
            case 'a':  // 缩写星期名称
                insertString(get_weekday_abbr_name(dateTime.wday));
                break;
            case '%':
                timeString.append("%");
                break;
            case '3':
                if (formatIdx + 1 < format.length() && format[formatIdx + 1] == 'f') {
                    formatIdx++;
                    std::format_to(std::back_inserter(timeString), "{:03}", dateTime.millis);
                } else {
                    timeString.append(std::string_view(format.data() + formatIdx - 1, 1));
                }
                break;
            default:
                timeString.append(std::string_view(format.data() + formatIdx - 1, 1));
                break;
        }
        if (!success) {
            break;
        }
    }

    set_thread_last_err(ERR_COMM_SUCCESS);
}

size_t format_time_buffer(char* buffer, size_t bufferSize, TimestampMs timestamp,
                          const std::string_view& format, TimeZone timeZone)
{
    auto dateTime = timestamp_to_date_time(timestamp, timeZone);
    return format_time_buffer(buffer, bufferSize, dateTime, format);
}

size_t format_time_buffer(char* buffer, size_t bufferSize, const DateTimeSt& dateTime,
                          const std::string_view& format)
{
    if (buffer == nullptr || bufferSize == 0) {
        ORIGIN_DEBUG_ERR("Invalid param!");
        return 0;
    }
    size_t formatIdx = 0;
    size_t bufferIdx = 0;

    //  lambda函数：将数字按指定长度写入缓冲区（自动补前导零）
    //  @param number    待写入的数字（如月份1 → 01）
    //  @param numberLen 数字固定长度（如2位、3位）
    //  @return 成功返回true；缓冲区不足返回false
    auto insertDateTimeNumber = [&](uint32_t number, size_t numberLen) -> bool {
        uint32_t tmp = number;
        if (bufferIdx + numberLen >= bufferSize) {
            set_thread_last_err(ERR_COMM_BUFFER_OVERFLOW);
            ORIGIN_DEBUG_ERR("Failed to insert number: %lu", number);
            return false;
        }

        size_t currIdx = bufferIdx + numberLen;
        while (currIdx > bufferIdx) {
            buffer[currIdx - 1] = static_cast<char>(tmp % 10 + '0');
            tmp = tmp / 10;
            currIdx--;
        }
        bufferIdx += numberLen;
        return true;
    };

    auto insertString = [&](const std::string_view& name) -> bool {
        std::string_view insertName = name.empty() ? "?" : name;

        if (bufferIdx + insertName.length() >= bufferSize) {
            set_thread_last_err(ERR_COMM_BUFFER_OVERFLOW);
            ORIGIN_DEBUG_ERR("Failed to insert string: {}", insertName.data());
            return false;
        }

        std::memcpy(buffer + bufferIdx, insertName.data(), insertName.size());
        bufferIdx += insertName.size();
        return true;
    };

    for (; bufferIdx < bufferSize && formatIdx < format.length(); formatIdx++) {
        if (format[formatIdx] != '%') {
            buffer[bufferIdx++] = format[formatIdx];
            continue;
        }
        formatIdx++;
        if (formatIdx >= format.length()) {
            buffer[bufferIdx++] = '%';
            break;
        }
        bool success = true;
        switch (format[formatIdx]) {
            case 'Y':
                success = insertDateTimeNumber(dateTime.year, 4);
                break;
            case 'y':
                success = insertDateTimeNumber(dateTime.year, 2);
                break;
            case 'm':
                success = insertDateTimeNumber(dateTime.month, 2);
                break;
            case 'd':
                success = insertDateTimeNumber(dateTime.day, 2);
                break;
            case 'H':
                success = insertDateTimeNumber(dateTime.hour, 2);
                break;
            case 'M':
                success = insertDateTimeNumber(dateTime.minute, 2);
                break;
            case 'S':
                success = insertDateTimeNumber(dateTime.second, 2);
                break;
            case 'B':  // 完整月份名称
                success = insertString(get_month_full_name(dateTime.month));
                break;
            case 'b':  // 缩写月份名称
            case 'h':  // 缩写月份名称
                success = insertString(get_month_abbr_name(dateTime.month));
                break;
            case 'A':  // 完整星期名称
                success = insertString(get_weekday_full_name(dateTime.wday));
                break;
            case 'a':  // 缩写星期名称
                success = insertString(get_weekday_abbr_name(dateTime.wday));
                break;
            case '%':
                buffer[bufferIdx++] = '%';
                break;
            case '3':
                if (formatIdx + 1 < format.length() && format[formatIdx + 1] == 'f') {
                    formatIdx++;
                    success = insertDateTimeNumber(dateTime.millis, 3);
                } else {
                    success = insertString(std::string_view(format.data() + formatIdx - 1, 1));
                }
                break;
            default:
                success = insertString(std::string_view(format.data() + formatIdx - 1, 1));
                break;
        }
        if (!success) {
            break;
        }
    }
    if (formatIdx < format.length() || bufferIdx >= bufferSize) {
        ORIGIN_DEBUG_ERR("Incomplete format processing (remaining: {}), message: {}",
                         format.data() + formatIdx,
                         get_thread_last_err_msg());
        bufferIdx = 0;
    } else {
        set_thread_last_err(ERR_COMM_SUCCESS);
    }

    buffer[bufferIdx] = '\0';

    return bufferIdx;
}

}  // namespace origin::date_time
