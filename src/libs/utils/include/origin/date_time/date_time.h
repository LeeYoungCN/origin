/**
 * @file date_time.h
 * @brief 时间工具库核心接口声明，提供时间获取、转换、格式化及日期名称查询功能
 * @details 跨平台支持Windows/Linux/macOS，封装底层系统差异，提供统一时间处理接口
 * @author 自动生成
 * @version 1.0.0
 * @date 2025-07-27
 * @copyright Copyright (c) 2025
 */
#ifndef ORIGIN_DATE_TIME_DATE_TIME_H
#define ORIGIN_DATE_TIME_DATE_TIME_H

#include <cstdint>
#include <ctime>
#include <string>
#include <string_view>

#include "origin/common/constants/date_time_constants.h"
#include "origin/common/types/date_time_types.h"
#include "origin/common/types/error_code_types.h"

namespace origin::date_time {
/**
 * @brief 获取当前系统时间的毫秒级时间戳
 * @details 以Unix纪元（1970-01-01 00:00:00 UTC）为基准，返回当前时间的毫秒数
 * @return TimestampMs 毫秒级时间戳
 * @note 跨平台实现：Windows使用GetSystemTimeAsFileTime，Linux/macOS使用std::chrono
 */
TimestampMs get_now_timestamp_ms();

/**
 * @brief 获取当前系统时间的时间组件
 * @details
 * 直接返回分解后的时间信息（年、月、日等），等价于Timestamp2Component(get_now_timestamp_ms())
 * @return DateTimeSt 当前时间的分解结构
 */
DateTimeSt get_now_date_time();

/**
 * @brief 将毫秒级时间戳转换为本地时间的时间组件
 * @param[in] timestamp 待转换的毫秒级时间戳（Unix纪元基准）
 * @param[out] out 转换后的时间组件；失败时所有字段置0
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
ErrorCode local_date_time(TimestampMs timestamp, DateTimeSt& out);

/**
 * @brief 将毫秒级时间戳转换为UTC时间的时间组件
 * @param[in] timestamp 待转换的毫秒级时间戳（Unix纪元基准）
 * @param[out] out 转换后的时间组件；失败时所有字段置0
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
ErrorCode utc_date_time(TimestampMs timestamp, DateTimeSt& out);

/**
 * @brief 将毫秒级时间戳转换为时间组件
 * @param[in] timestamp 待转换的毫秒级时间戳（Unix纪元基准）
 * @param[in] timeZone 时区（LOCAL / UTC）
 * @param[out] out 转换后的时间组件；失败时所有字段置0
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回 ERR_UTILS_TIMESTAMP_INVALID 等错误码
 */
ErrorCode timestamp_to_date_time(TimestampMs timestamp, TimeZone timeZone, DateTimeSt& out);

/**
 * @brief 将时间组件转换为毫秒级时间戳
 * @param[in] dateTime 待转换的时间组件（按本地时区解释）
 * @param[out] out 转换后的毫秒级时间戳；失败时置 -1
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回 ERR_UTILS_TIMESTAMP_INVALID
 */
ErrorCode date_time_to_timestamp(const DateTimeSt& dateTime, TimestampMs& out);

/**
 * @brief 比较两个毫秒级时间戳之间毫秒级时间差
 * @return DurationMs 毫秒级时间差
 */
DurationMs diff_timestamp(TimestampMs x, TimestampMs y);

/**
 * @brief 比较两个时间组件之间毫秒级时间差
 * @return DurationMs 毫秒级时间差
 */
DurationMs diff_date_time(const DateTimeSt& x, const DateTimeSt& y);

/**
 * @brief 获取月份的完整英文名称
 * @param[in] month 月份（1-12，1表示一月，12表示十二月）
 * @param[out] out 完整月份名称（如"January"）；失败时为空字符串视图
 * @return 成功返回 ERR_COMM_SUCCESS，参数越界返回 ERR_UTILS_MONTH_INVALID
 */
ErrorCode get_month_full_name(uint32_t month, std::string_view& out);

/**
 * @brief 获取月份的缩写英文名称
 * @param[in] month 月份（1-12）
 * @param[out] out 缩写月份名称（如"Jan"）；失败时为空字符串视图
 * @return 成功返回 ERR_COMM_SUCCESS，参数越界返回 ERR_UTILS_MONTH_INVALID
 */
ErrorCode get_month_abbr_name(uint32_t month, std::string_view& out);

/**
 * @brief 获取星期的完整英文名称
 * @param[in] weekday 星期（0-6，0表示星期日，6表示星期六）
 * @param[out] out 完整星期名称（如"Monday"）；失败时为空字符串视图
 * @return 成功返回 ERR_COMM_SUCCESS，参数越界返回 ERR_UTILS_WEEKDAY_INVALID
 */
ErrorCode get_weekday_full_name(uint32_t weekday, std::string_view& out);

/**
 * @brief 获取星期的缩写英文名称
 * @param[in] weekday 星期（0-6）
 * @param[out] out 缩写星期名称（如"Mon"）；失败时为空字符串视图
 * @return 成功返回 ERR_COMM_SUCCESS，参数越界返回 ERR_UTILS_WEEKDAY_INVALID
 */
ErrorCode get_weekday_abbr_name(uint32_t weekday, std::string_view& out);

/**
 * @brief 将时间戳按指定格式转换为字符串
 * @param[in] timestamp 待格式化的毫秒级时间戳
 * @param[in] format 格式字符串（支持占位符，如%Y表示4位年份）；
 *                   常用格式见 origin::constants::date_time::DEFAULT_TIME_FMT
 *                   （"%Y-%m-%d %H:%M:%S"），需显式传入
 * @param[in] timeZone 时区（LOCAL / UTC）
 * @param[out] out 格式化结果；函数进入时先清空
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
ErrorCode format_time_string(TimestampMs timestamp,
                             const std::string_view& format,
                             TimeZone timeZone,
                             std::string& out);

/**
 * @brief 将时间组件按指定格式转换为字符串
 * @param[in] dateTime 待格式化的时间组件
 * @param[in] format 格式字符串（支持占位符，如%Y表示4位年份）；
 *                   常用格式见 origin::constants::date_time::DEFAULT_TIME_FMT
 *                   （"%Y-%m-%d %H:%M:%S"），需显式传入
 * @param[out] out 格式化结果；函数进入时先清空
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
ErrorCode format_time_string(const DateTimeSt& dateTime,
                             const std::string_view& format,
                             std::string& out);

/**
 * @brief 将时间戳按指定格式转换为字符串并追加到已有字符串中
 * @details 适用于高频调用场景，避免频繁内存分配
 * @param[in,out] timeString 已存在的字符串，格式化结果追加到末尾
 * @param[in] timestamp 待格式化的毫秒级时间戳
 * @param[in] format 格式字符串（支持占位符，如%Y表示4位年份）；
 *                   常用格式见 origin::constants::date_time::DEFAULT_TIME_FMT
 *                   （"%Y-%m-%d %H:%M:%S"），需显式传入
 * @param[in] timeZone 时区（LOCAL / UTC）
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
ErrorCode append_time_string(std::string& timeString,
                             TimestampMs timestamp,
                             const std::string_view& format,
                             TimeZone timeZone);

/**
 * @brief 将时间组件按指定格式转换为字符串并追加到已有字符串中
 * @details 适用于高频调用场景，避免频繁内存分配
 * @param[in,out] timeString 已存在的字符串，格式化结果追加到末尾
 * @param[in] dateTime 待格式化的时间组件
 * @param[in] format 格式字符串（支持占位符，如%Y表示4位年份）；
 *                   常用格式见 origin::constants::date_time::DEFAULT_TIME_FMT
 *                   （"%Y-%m-%d %H:%M:%S"），需显式传入
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
ErrorCode append_time_string(std::string& timeString,
                             const DateTimeSt& dateTime,
                             const std::string_view& format);

/**
 * @brief 将时间戳按指定格式写入字符缓冲区（高性能）
 * @details 直接写入用户提供的缓冲区，减少内存分配，适合高频调用场景
 * @param[out] buffer 目标缓冲区（需提前分配内存，不可为nullptr）
 * @param[in] bufferSize 缓冲区大小（字节），建议不小于256
 * @param[in] timestamp 待格式化的毫秒级时间戳
 * @param[in] format 格式字符串（支持占位符）；常用格式见
 *                   origin::constants::date_time::DEFAULT_TIME_FMT
 *                   （"%Y-%m-%d %H:%M:%S"），需显式传入
 * @param[in] timeZone 时区（LOCAL / UTC）
 * @param[out] outLen 实际写入的字符数（不含终止符'\0'）；失败时为0
 * @return 成功返回 ERR_COMM_SUCCESS；参数无效返回 ERR_COMM_PARAM_INVALID，
 *         缓冲区不足返回 ERR_COMM_BUFFER_OVERFLOW
 */
ErrorCode format_time_buffer(char* buffer,
                             size_t bufferSize,
                             TimestampMs timestamp,
                             const std::string_view& format,
                             TimeZone timeZone,
                             size_t& outLen);

/**
 * @brief 将时间组件按指定格式写入字符缓冲区（高性能）
 * @details 直接写入用户提供的缓冲区，减少内存分配，适合高频调用场景
 * @param[out] buffer 目标缓冲区（需提前分配内存，不可为nullptr）
 * @param[in] bufferSize 缓冲区大小（字节），建议不小于256
 * @param[in] dateTime 待格式化的时间组件
 * @param[in] format 格式字符串（支持占位符）；常用格式见
 *                   origin::constants::date_time::DEFAULT_TIME_FMT
 *                   （"%Y-%m-%d %H:%M:%S"），需显式传入
 * @param[out] outLen 实际写入的字符数（不含终止符'\0'）；失败时为0
 * @return 成功返回 ERR_COMM_SUCCESS；参数无效返回 ERR_COMM_PARAM_INVALID，
 *         缓冲区不足返回 ERR_COMM_BUFFER_OVERFLOW
 */
ErrorCode format_time_buffer(char* buffer,
                             size_t bufferSize,
                             const DateTimeSt& dateTime,
                             const std::string_view& format,
                             size_t& outLen);

// ------------------------------ 系统时间操作 ------------------------------

/**
 * @brief 线程睡眠指定毫秒数
 * @param ms 睡眠时长（毫秒）
 */
void sleep_ms(DurationMs ms);

}  // namespace origin::date_time
#endif  // ORIGIN_DATE_TIME_DATE_TIME_H
