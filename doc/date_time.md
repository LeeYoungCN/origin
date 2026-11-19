# 日期时间工具（`origin::date_time`）

跨平台（Windows / Linux / macOS）的时间处理接口：获取当前时间、时间戳与时间组件互转、
格式化、日期名称查询。

## 头文件与命名空间

| 内容 | 头文件 | 命名空间 |
|---|---|---|
| 时间类型（`TimestampMs`、`DateTimeSt` 等） | `origin/common/types/date_time_types.h` | **全局命名空间** |
| 时间常量 | `origin/common/constants/date_time_constants.h` | `origin::date_time` |
| 工具接口 | `origin/date_time/date_time.h` | `origin::date_time` |

> 类型定义在全局命名空间，不在 `origin::date_time` 里；`TimestampMs`、`DateTimeSt`
> 直接写名字即可。

## 类型

```cpp
using TimestampSec = int64_t;   // 秒级时间戳
using TimestampMs  = int64_t;   // 毫秒级时间戳（Unix epoch 基准）
using DurationSec  = int64_t;   // 秒级间隔
using DurationMs   = int64_t;   // 毫秒级间隔

struct DateTimeSt {
    uint32_t year = 0;    // 4 位年份
    uint32_t month = 0;   // 1-12
    uint32_t day = 0;     // 1-31
    uint32_t hour = 0;    // 0-23
    uint32_t minute = 0;  // 0-59
    uint32_t second = 0;  // 0-59
    uint32_t millis = 0;  // 0-999
    uint32_t wday = 0;    // 0-6，具体对应关系取决于系统（通常 0 为周日）
    uint32_t yday = 0;    // 年内天数，0-365
};

typedef enum {
    UTC,    // 世界协调时间
    LOCAL,  // 本地时区
} TimeZone;
```

## 常用常量（`origin::date_time`）

| 常量 | 值 | 说明 |
|---|---|---|
| `DEFAULT_TIME_FMT` | `"%Y-%m-%d %H:%M:%S"` | 默认格式 |
| `DEFAULT_TIME_FMT_MS` | `"%Y-%m-%d %H:%M:%S.%3f"` | 带毫秒的默认格式 |
| `MAX_TIME_STR_LEN` | `256` | 时间字符串建议缓冲长度 |
| `MILLIS_PER_SEC` | `1000` | 单位换算 |
| `UNIX_EPOCH_MS` | `0` | Unix epoch 毫秒值 |

其余换算常量（`MILLIS_PER_MINUTE`、`MILLIS_PER_HOUR`、`MILLIS_PER_DAY` 等）
与范围常量（`MIN_MONTH`、`MAX_MONTH`、`MIN_WEEKDAY`、`MAX_WEEKDAY` 等）见头文件。

## 接口

### 获取当前时间

| 接口 | 说明 |
|---|---|
| `TimestampMs get_now_timestamp_ms()` | 当前毫秒时间戳 |
| `DateTimeSt get_now_date_time()` | 当前本地时间组件 |

### 时间转换

| 接口 | 说明 |
|---|---|
| `DateTimeSt local_date_time(TimestampMs)` | 时间戳 → 本地时间组件 |
| `DateTimeSt utc_date_time(TimestampMs)` | 时间戳 → UTC 时间组件 |
| `DateTimeSt timestamp_to_date_time(TimestampMs, TimeZone = LOCAL)` | 指定时区 |
| `TimestampMs date_time_to_timestamp(const DateTimeSt&)` | 本地时间组件 → 时间戳 |

### 计算与等待

| 接口 | 说明 |
|---|---|
| `DurationMs diff_timestamp(TimestampMs x, TimestampMs y)` | 两个时间戳之差 |
| `DurationMs diff_date_time(const DateTimeSt& x, const DateTimeSt& y)` | 两个时间组件之差 |
| `void sleep_ms(DurationMs ms)` | 毫秒级睡眠 |

### 日期名称

| 接口 | 说明 |
|---|---|
| `get_month_full_name(uint32_t month)` | `"January"` …；参数越界返回空 `string_view` |
| `get_month_abbr_name(uint32_t month)` | `"Jan"` … |
| `get_weekday_full_name(uint32_t weekday)` | `"Monday"` … |
| `get_weekday_abbr_name(uint32_t weekday)` | `"Mon"` … |

### 格式化

```cpp
std::string format_time_string(TimestampMs timestamp, const std::string_view& format);
std::string format_time_string(const DateTimeSt& dateTime, const std::string_view& format);

void append_time_string(std::string& timeString, TimestampMs timestamp,
                        const std::string_view& format);
void append_time_string(std::string& timeString, const DateTimeSt& dateTime,
                        const std::string_view& format);

size_t format_time_buffer(char* buffer, size_t bufferSize, TimestampMs timestamp,
                          const std::string_view& format);
size_t format_time_buffer(char* buffer, size_t bufferSize, const DateTimeSt& dateTime,
                          const std::string_view& format);
```

| 占位符 | 含义 | 示例 |
|---|---|---|
| `%Y` | 4 位年份 | `2026` |
| `%y` | 2 位年份 | `26` |
| `%m` | 2 位月份 | `11` |
| `%d` | 2 位日期 | `17` |
| `%H` | 2 位小时（24 小时制） | `15` |
| `%M` | 2 位分钟 | `30` |
| `%S` | 2 位秒 | `45` |
| `%B` | 完整月份名 | `November` |
| `%b` / `%h` | 缩写月份名 | `Nov` |
| `%A` | 完整星期名 | `Monday` |
| `%a` | 缩写星期名 | `Mon` |
| `%3f` | 3 位毫秒 | `678` |
| `%%` | 百分号 | `%` |

## 示例

```cpp
#include <cstdio>

#include <origin/date_time/date_time.h>

int main()
{
    const TimestampMs now = origin::date_time::get_now_timestamp_ms();
    const DateTimeSt dt = origin::date_time::local_date_time(now);

    const auto text = origin::date_time::format_time_string(dt, "%Y-%m-%d %H:%M:%S.%3f");
    std::printf("%s\n", text.c_str());

    char buffer[origin::date_time::MAX_TIME_STR_LEN];
    const size_t len =
        origin::date_time::format_time_buffer(buffer, sizeof(buffer), dt, "%Y/%m/%d %H:%M");
    if (len > 0) {
        std::printf("%s\n", buffer);
    }
    return 0;
}
```

## 注意事项

- 格式化失败返回空字符串（`format_time_buffer` 返回 `0`）；
- `format_time_buffer` 的 `buffer` 不能为 `nullptr`，`bufferSize` 必须大于 0，
  建议至少 `MAX_TIME_STR_LEN`；
- `local_date_time` 转换失败时返回字段全为 0 的 `DateTimeSt`（`year == 0` 可判定）；
- 日期名称固定为英文，不做本地化；
- `wday` / `yday` 的取值来自底层系统实现，跨平台使用时以实际测试为准。
