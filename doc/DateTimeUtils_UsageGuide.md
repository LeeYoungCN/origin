# 时间工具库（DateTime Utils）完整用户手册


## 概述

时间工具库是一套跨平台（Windows/Linux/macOS）的C++时间处理工具，提供**时间获取、时间转换、格式化及日期名称查询**等功能。通过封装底层实现细节，以简洁接口支持将时间戳或时间组件转换为自定义格式的字符串，适用于日志记录、数据展示、时间计算等场景。


## 核心文件结构

| 文件路径 | 功能描述 |
|----------|----------|
| `common/types/date_time_types.h` | 定义基础时间类型（`TimestampMs`、`TimeComponent`） |
| `common/constants/date_time_constants.h` | 定义时间相关常量（如单位转换系数、范围限制） |
| `origin/date_time/date_time.h` | 声明所有工具接口（时间获取、转换、格式化等） |


## 一、数据类型

所有时间类型定义于 `common/types/date_time_types.h`，位于命名空间 `common::types::date_time`。


### 1.1 `TimestampMs`（时间戳）

```cpp
// 定义于 common/types/date_time_types.h
namespace origin::common::types::date_time {
    using TimestampMs = int64_t;  ///< 毫秒级时间戳，以Unix纪元（1970-01-01 00:00:00 UTC）为基准
}
```

- **取值范围**：正负整数（正值表示纪元后时间，负值表示纪元前时间）
- **用途**：高效存储和传递时间（如日志时间戳、事件时间标记）


### 1.2 `TimeComponent`（时间组件）

```cpp
// 定义于 common/types/date_time_types.h
namespace origin::common::types::date_time {
    struct TimeComponent {
        uint32_t year = 0;    ///< 4位完整年份（如2024）
        uint32_t month = 0;   ///< 月份（1-12，1表示一月，12表示十二月）
        uint32_t day = 0;     ///< 日期（1-31，当月第几天）
        uint32_t hour = 0;    ///< 小时（0-23，24小时制）
        uint32_t minute = 0;  ///< 分钟（0-59）
        uint32_t second = 0;  ///< 秒（0-59）
        uint32_t millis = 0;  ///< 毫秒（0-999）
        uint32_t wday = 0;    ///< 星期（0-6，0表示星期日，6表示星期六）
        uint32_t yday = 0;    ///< 年内天数（0-365，0表示1月1日）
    };
}
```

- **用途**：存储人类可读的分解时间信息，用于格式化和展示
- **关联关系**：通常由 `TimestampMs` 通过 `local_date_time`、 `utc_time_info` 转换生成


## 二、核心常量

时间相关常量定义于 `common/constants/date_time_constants.h`，位于命名空间 `origin::date_time`，关键常量如下：

| 常量 | 类型 | 说明 |
|------|------|------|
| `MILLIS_PER_SECOND` | `int64_t` | 1秒=1000毫秒（时间单位转换用） |
| `MIN_MONTH` / `MAX_MONTH` | `uint32_t` | 月份有效范围（1-12） |
| `MIN_WEEK_DAY` / `MAX_WEEK_DAY` | `uint32_t` | 星期有效范围（0-6） |
| `MAX_TIME_STR_LEN` | `uint32_t` | 时间字符串最大长度（256，用于缓冲区初始化） |
| `DEFAULT_TIME_FMT` | `const char*` | 默认时间格式（`"%Y-%m-%d %H:%M:%S"`） |


## 三、工具接口

所有接口声明于 `origin/date_time/date_time.h`，位于命名空间 `origin::date_time`。


### 3.1 时间获取接口

#### 3.1.1 `get_now_timestamp_ms`

```cpp
/**
 * @brief 获取当前系统时间的毫秒级时间戳
 * 
 * @return common::types::date_timeTimestampMs 
 *         当前时间戳（Unix纪元基准，如1722057600000表示2024-07-27 00:00:00）
 */
common::types::date_timeTimestampMs get_now_timestamp_ms();
```


#### 3.1.2 `get_now_date_time`

```cpp
/**
 * @brief 获取当前系统时间的时间组件
 * 
 * @return common::types::date_time::TimeComponent 
 *         当前时间的分解信息（年、月、日、时、分、秒等）
 */
common::types::date_time::TimeComponent get_now_date_time();
```


### 3.2 时间转换接口

#### 3.2.1 `local_date_time`

```cpp
/**
 * @brief 将毫秒级时间戳转换为本地时间的时间组件
 * 
 * @param timestamp 待转换的时间戳（`common::types::date_timeTimestampMs` 类型）
 * @return common::types::date_time::TimeComponent 
 *         转换后的时间组件（失败时返回字段全为0的结构）
 */
common::types::date_time::TimeComponent local_date_time(
    const common::types::date_timeTimestampMs& timestamp
);
```


#### 3.2.2 `utc_time_info`

```cpp
/**
 * @brief 将毫秒级时间戳转换为UTC时间的时间组件
 * 
 * @param timestamp 待转换的时间戳（`common::types::date_timeTimestampMs` 类型）
 * @return common::types::date_time::TimeComponent 
 *         转换后的时间组件（失败时返回字段全为0的结构）
 */
common::types::date_time::TimeComponent utc_time_info(
    const common::types::date_timeTimestampMs& timestamp
);
```


### 3.3 日期名称接口

| 接口 | 功能 | 参数要求 | 返回值 |
|------|------|----------|--------|
| `get_month_full_name` | 获取月份完整英文名称（如“January”） | 月份：1-12（1=一月） | 名称的`std::string_view`（参数无效返回空） |
| `get_month_abbr_name` | 获取月份缩写英文名称（如“Jan”） | 月份：1-12 | 名称的`std::string_view`（参数无效返回空） |
| `get_weekday_full_name` | 获取星期完整英文名称（如“Monday”） | 星期：0-6（0=星期日） | 名称的`std::string_view`（参数无效返回空） |
| `get_weekday_abbr_name` | 获取星期缩写英文名称（如“Mon”） | 星期：0-6 | 名称的`std::string_view`（参数无效返回空） |


### 3.4 时间格式化接口

#### 3.4.1 字符串格式化（`format_time_string`）

```cpp
/**
 * @brief 按指定格式将时间戳转换为字符串
 * 
 * @param timestamp 待格式化的时间戳（`common::types::date_timeTimestampMs` 类型）
 * @param format 格式字符串（支持占位符，见3.4.3节）
 * @return std::string 格式化后的字符串（失败返回空字符串）
 */
std::string format_time_string(
    common::types::date_timeTimestampMs timestamp, 
    const std::string_view& format
);

/**
 * @brief 按指定格式将时间组件转换为字符串
 * 
 * @param dateTime 待格式化的时间组件（`common::types::date_time::TimeComponent` 类型）
 * @param format 格式字符串
 * @return std::string 格式化后的字符串（失败返回空字符串）
 */
std::string format_time_string(
    const common::types::date_time::TimeComponent& dateTime, 
    const std::string_view& format
);
```


#### 3.4.2 缓冲区格式化（`format_time_buffer`）

```cpp
/**
 * @brief 按指定格式将时间写入字符缓冲区（高性能，减少内存分配）
 * 
 * @param buffer 目标缓冲区（需提前分配，不可为nullptr）
 * @param bufferSize 缓冲区大小（字节，建议≥256）
 * @param timestamp / dateTime 待格式化的时间
 * @param format 格式字符串
 * @return size_t 成功写入的字符数（不含终止符`\0`）；失败返回0
 */
size_t format_time_buffer(
    char* buffer, 
    size_t bufferSize, 
    common::types::date_timeTimestampMs timestamp, 
    const std::string_view& format
);

size_t format_time_buffer(
    char* buffer, 
    size_t bufferSize, 
    const common::types::date_time::TimeComponent& dateTime, 
    const std::string_view& format
);
```


#### 3.4.3 格式占位符说明

| 占位符 | 描述 | 示例 |
|--------|------|------|
| `%Y` | 4位年份 | 2024 |
| `%y` | 2位年份（取后两位） | 24 |
| `%m` | 2位月份（补零） | 07（7月） |
| `%d` | 2位日期（补零） | 27 |
| `%H` | 24小时制小时（补零） | 15（下午3点） |
| `%M` | 2位分钟（补零） | 30 |
| `%S` | 2位秒数（补零） | 45 |
| `%B` | 完整月份名称 | July |
| `%b`/`%h` | 缩写月份名称 | Jul |
| `%A` | 完整星期名称 | Saturday |
| `%a` | 缩写星期名称 | Sat |
| `%3f` | 3位毫秒（补零） | 678 |
| `%%` | 百分号本身 | % |


## 四、头文件引用及命名空间

### 4.1 头文件引用

| 需求 | 必须引用的头文件 |
|------|------------------|
| 使用 `TimestampMs`/`TimeComponent` 类型 | `#include "common/types/date_time_types.h"` |
| 调用所有工具接口（获取、转换、格式化等） | `#include "origin/date_time/date_time.h"` |


### 4.2 命名空间

| 命名空间 | 包含内容 | 示例 |
|----------|----------|------|
| `common::types::date_time` | `TimestampMs`、`TimeComponent` 类型定义 | `common::types::date_timeTimestampMs` |
| `origin::date_time` | 时间相关常量（如 `MILLIS_PER_SECOND`） | `origin::date_time::MAX_TIME_STR_LEN` |
| `origin::date_time` | 所有工具接口（`get_now_timestamp_ms` 等） | `origin::date_time::format_time_string` |


### 4.3 命名空间使用建议

- 在 `.cpp` 文件中，可通过 `using` 声明简化代码：

  ```cpp
  using namespace origin::common::types::date_time;    // 时间类型
  using namespace origin::origin::date_time;    // 工具接口
  ```

- 在头文件中，避免使用 `using namespace`，需使用完整命名空间（如 `common::types::date_timeTimestampMs`）。


## 五、使用示例

### 5.1 基础用法：获取并格式化当前时间

```cpp
// 头文件引用
#include "common/types/date_time_types.h"
#include "origin/date_time/date_time.h"
#include <iostream>

int main() {
    // 简化命名空间
    using namespace origin::common::types::date_time;
    using namespace origin::origin::date_time;

    // 1. 获取当前时间戳
    TimestampMs now_ts = get_now_timestamp_ms();
    std::cout << "当前时间戳: " << now_ts << " ms" << std::endl;

    // 2. 转换为时间组件
    TimeComponent now_comp = local_date_time(now_ts);
    std::cout << "当前时间: " << now_comp.year << "-" 
              << now_comp.month << "-" << now_comp.day << std::endl;

    // 3. 格式化时间
    std::string fmt_str = format_time_string(now_comp, "%Y-%m-%d %H:%M:%S.%3f %A");
    if (!fmt_str.empty()) {
        std::cout << "格式化结果: " << fmt_str << std::endl;
        // 输出示例：2024-07-27 15:30:45.678 Saturday
    }

    return 0;
}
```


### 5.2 进阶用法：时间组件与日期名称

```cpp
int main() {
    using namespace origin::common::types::date_time;
    using namespace origin::origin::date_time;

    // 定义时间组件（2023年1月1日 00:00:00.500，星期日）
    TimeComponent comp = {2023, 1, 1, 0, 0, 0, 500, 0};

    // 获取月份和星期名称
    auto month_name = get_month_full_name(comp.month);    // "January"
    auto weekday_name = get_weekday_abbr_name(comp.wday); // "Sun"
    std::cout << "月份: " << month_name << ", 星期: " << weekday_name << std::endl;

    // 缓冲区格式化（高性能场景）
    char buffer[256];
    size_t len = format_time_buffer(
        buffer, sizeof(buffer), comp, "%b %d, %Y %H:%M:%S.%3f"
    );
    if (len > 0) {
        std::cout << "缓冲区结果: " << buffer << std::endl;  // Jan 01, 2023 00:00:00.500
    }

    return 0;
}
```


## 六、注意事项

1. **参数有效性**：
   - 月份需在1-12范围内，星期需在0-6范围内，否则日期名称接口返回空。
   - 调用 `format_time_buffer` 时，`buffer` 不可为 `nullptr`，`bufferSize` 需>0。

2. **错误判断**：
   - 格式化接口返回空字符串（或0）时，表示失败。
   - `local_date_time` 返回 `year == 0` 时，表示转换失败。

3. **性能建议**：
   - 高频格式化场景（如日志），优先使用 `format_time_buffer`（减少字符串分配）。
   - 缓冲区大小建议≥256字节（`MAX_TIME_STR_LEN`），避免空间不足。

4. **本地化**：
   - 日期名称默认返回英文，如需其他语言，需扩展接口实现。


## 七、兼容性

- 支持平台：Windows、Linux、macOS
- 依赖：C++17及以上标准


通过本手册，可快速掌握时间工具库的使用方法，实现时间获取、转换和格式化功能，无需关注底层实现细节。
