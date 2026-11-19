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
| `DEFAULT_TIME_FMT` | `"%Y-%m-%d %H:%M:%S"` | 常用格式，需显式传给格式化接口 |
| `DEFAULT_TIME_FMT_MS` | `"%Y-%m-%d %H:%M:%S.%3f"` | 带毫秒的常用格式，需显式传入 |
| `MAX_TIME_STR_LEN` | `256` | 时间字符串建议缓冲长度 |
| `MILLIS_PER_SEC` | `1000` | 单位换算 |
| `UNIX_EPOCH_MS` | `0` | Unix epoch 毫秒值 |

其余换算常量（`MILLIS_PER_MINUTE`、`MILLIS_PER_HOUR`、`MILLIS_PER_DAY` 等）
与范围常量（`MIN_MONTH`、`MAX_MONTH`、`MIN_WEEKDAY`、`MAX_WEEKDAY` 等）见头文件。

## 接口

可失败接口统一返回 `ErrorCode`，需要产出值的接口用末尾出参。

### 获取当前时间

| 接口 | 说明 |
|---|---|
| `TimestampMs get_now_timestamp_ms()` | 当前毫秒时间戳 |
| `DateTimeSt get_now_date_time()` | 当前本地时间组件 |

### 时间转换

| 接口 | 说明 |
|---|---|
| `ErrorCode local_date_time(TimestampMs, DateTimeSt& out)` | 时间戳 → 本地时间组件 |
| `ErrorCode utc_date_time(TimestampMs, DateTimeSt& out)` | 时间戳 → UTC 时间组件 |
| `ErrorCode timestamp_to_date_time(TimestampMs, TimeZone, DateTimeSt& out)` | 指定时区转换（`timeZone` 无默认值） |
| `ErrorCode date_time_to_timestamp(const DateTimeSt&, TimestampMs& out)` | 本地时间组件 → 时间戳；失败时 `out = -1` |

### 计算与等待

| 接口 | 说明 |
|---|---|
| `DurationMs diff_timestamp(TimestampMs x, TimestampMs y)` | 两个时间戳之差 |
| `DurationMs diff_date_time(const DateTimeSt& x, const DateTimeSt& y)` | 两个时间组件之差 |
| `void sleep_ms(DurationMs ms)` | 毫秒级睡眠 |

### 日期名称

| 接口 | 说明 |
|---|---|
| `ErrorCode get_month_full_name(uint32_t month, std::string_view& out)` | `"January"` …；越界返回 `ERR_UTILS_MONTH_INVALID` |
| `ErrorCode get_month_abbr_name(uint32_t month, std::string_view& out)` | `"Jan"` … |
| `ErrorCode get_weekday_full_name(uint32_t weekday, std::string_view& out)` | `"Monday"` …；越界返回 `ERR_UTILS_WEEKDAY_INVALID` |
| `ErrorCode get_weekday_abbr_name(uint32_t weekday, std::string_view& out)` | `"Mon"` … |

### 格式化

**值返回版（忽略错误码）**：直接拿结果，失败时返回空串 / `0`；`format` 默认
`DEFAULT_TIME_FMT`，`timeZone` 默认 `LOCAL`：

```cpp
std::string format_time_string(TimestampMs timestamp,
                               const std::string_view& format = DEFAULT_TIME_FMT,
                               TimeZone timeZone = LOCAL);
std::string format_time_string(const DateTimeSt& dateTime,
                               const std::string_view& format = DEFAULT_TIME_FMT);

size_t format_time_buffer(char* buffer, size_t bufferSize, TimestampMs timestamp,
                          const std::string_view& format = DEFAULT_TIME_FMT,
                          TimeZone timeZone = LOCAL);
size_t format_time_buffer(char* buffer, size_t bufferSize, const DateTimeSt& dateTime,
                          const std::string_view& format = DEFAULT_TIME_FMT);
```

**ErrorCode 版（需要区分错误原因时）**：没有默认参数，所有参数必须显式传入：

```cpp
ErrorCode format_time_string(TimestampMs timestamp, const std::string_view& format,
                             TimeZone timeZone, std::string& out);
ErrorCode format_time_string(const DateTimeSt& dateTime, const std::string_view& format,
                             std::string& out);

ErrorCode append_time_string(std::string& timeString, TimestampMs timestamp,
                             const std::string_view& format, TimeZone timeZone);
ErrorCode append_time_string(std::string& timeString, const DateTimeSt& dateTime,
                             const std::string_view& format);

ErrorCode format_time_buffer(char* buffer, size_t bufferSize, TimestampMs timestamp,
                             const std::string_view& format, TimeZone timeZone, size_t& outLen);
ErrorCode format_time_buffer(char* buffer, size_t bufferSize, const DateTimeSt& dateTime,
                             const std::string_view& format, size_t& outLen);
```

> 选择建议：只想要结果就用值返回版；需要区分“参数错误 / 缓冲区不足”时用 ErrorCode 版。
> `append_time_string` 没有值返回版本（原地追加），忽略错误码时写
> `(void)append_time_string(...)`。

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
#include <string>

#include <origin/common/constants/date_time_constants.h>
#include <origin/date_time/date_time.h>
#include <origin/utils/utils_error_code.h>

int main()
{
    const TimestampMs now = origin::date_time::get_now_timestamp_ms();

    DateTimeSt dt{};
    if (origin::date_time::local_date_time(now, dt) != ERR_COMM_SUCCESS) {
        return 1;
    }

    // 值返回版：忽略错误码，默认格式 + 默认时区
    const std::string text = origin::date_time::format_time_string(dt);
    std::printf("%s\n", text.c_str());

    // 值返回版：显式指定带毫秒的格式
    const std::string textMs =
        origin::date_time::format_time_string(dt, origin::date_time::DEFAULT_TIME_FMT_MS);
    std::printf("%s\n", textMs.c_str());

    // ErrorCode 版：需要区分错误原因时
    char buffer[origin::date_time::MAX_TIME_STR_LEN];
    size_t len = 0;
    const ErrorCode bufRc = origin::date_time::format_time_buffer(
        buffer, sizeof(buffer), dt, "%Y/%m/%d %H:%M", len);
    if (bufRc != ERR_COMM_SUCCESS) {
        std::printf("format buffer failed: %s\n", get_utils_err_msg(bufRc));
        return 1;
    }
    std::printf("%s\n", buffer);
    return 0;
}
```

## 注意事项

- 可失败接口返回 `ErrorCode`：成功为 `ERR_COMM_SUCCESS`，失败为具体错误码；
- 格式化接口有两套：值返回版（`format` 默认 `DEFAULT_TIME_FMT`、`timeZone` 默认
  `LOCAL`，失败返回空串/`0`）；ErrorCode 版（所有参数显式传入）；
- 格式化失败返回错误码（如 `ERR_COMM_BUFFER_OVERFLOW`、`ERR_COMM_PARAM_INVALID`），
  不再用空字符串或 `0` 表示失败；
- `format_time_buffer` 的 `buffer` 不能为 `nullptr`，`bufferSize` 必须大于 0，
  建议至少 `MAX_TIME_STR_LEN`；
- `local_date_time` 转换失败返回 `ERR_UTILS_TIMESTAMP_INVALID`，`out` 字段全为 0
  （`year == 0` 可判定）；
- 日期名称固定为英文，不做本地化；
- `wday` / `yday` 的取值来自底层系统实现，跨平台使用时以实际测试为准。
