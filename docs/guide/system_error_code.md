# 错误码与错误类别

本文分两部分：

1. Origin 自己的错误码体系（`ErrorCode` + 模块化宏 + 纯函数错误文本）；
2. C++ 标准库的 `std::error_category`（`generic_category` / `system_category`），
   以及它们与 Origin 错误码的对应关系。

## 一、Origin 错误码

### 1.1 位布局

`ErrorCode` 是 64 位无符号整数，按位拆成四段：

| 字段 | 位 | 掩码 | 说明 |
|---|---|---|---|
| severity | 63–48 | `ERR_SEVERITY_MASK` | 错误级别 |
| module | 47–32 | `ERR_MODULE_MASK` | 模块 ID |
| type | 31–16 | `ERR_TYPE_MASK` | 模块内错误类别 |
| id | 15–0 | `ERR_ID_MASK` | 具体错误编号 |

```cpp
typedef uint64_t ErrorCode;
typedef uint16_t ErrModuleId;
typedef uint16_t ErrorType;
typedef uint16_t ErrorId;

#define MAKE_ERROR(severity, module, type, errId)                                  \
    ((ErrorCode)(((uint64_t)(severity) << ERR_SEVERITY_SHIFT) |                    \
                 ((uint64_t)(module) << ERR_MODULE_SHIFT) |                        \
                 ((uint64_t)(type) << ERR_TYPE_SHIFT) |                            \
                 ((uint64_t)(errId) << ERR_ID_SHIFT)))
```

### 1.2 错误级别

| 枚举 | 值 | 含义 |
|---|---|---|
| `ERR_SEV_SUCCESS` | 0 | 成功 |
| `ERR_SEV_INFO` | 1 | 提示，不影响流程 |
| `ERR_SEV_WARN` | 2 | 警告，可能有风险 |
| `ERR_SEV_ERROR` | 3 | 错误，功能失败但可恢复 |
| `ERR_SEV_FATAL` | 4 | 严重错误，不可恢复 |

### 1.3 模块 ID

| 宏 | 值 |
|---|---|
| `COMMON_MODULE_ID` | 1 |
| `UTILS_MODULE_ID` | 2 |
| `CONTAINER_MODULE_ID` | 3 |

### 1.4 各模块的错误码宏

| 头文件 | 构造宏 | 示例 |
|---|---|---|
| `origin/common/common_error_code.h` | `MAKE_COMMON_ERROR(id)` | `ERR_COMM_PARAM_INVALID`、`ERR_COMM_MALLOC_FAILED`、`ERR_COMM_SUCCESS` |
| `origin/utils/utils_error_code.h` | `MAKE_DATE_TIME_ERROR(id)` / `MAKE_FILESYS_ERROR(id)` | `ERR_UTILS_MONTH_INVALID`、`ERR_UTILS_NOT_FOUND`、`ERR_UTILS_PERMISSION_DENIED` |
| `origin/container/container_error_code.h` | `MAKE_CONTAINER_ERROR(id)` | `ERR_CONTAINER_OVERFLOW` |

`ERR_COMM_SUCCESS` 的值为 `0`，所有非零值都表示某种错误。

### 1.5 错误信息

```cpp
#include <origin/common/common_error_code.h>

const char* text = get_comm_err_msg(errCode);   // 常见错误码 → 可读文本
```

### 1.6 utils 的错误返回

utils 的可失败接口统一返回 `ErrorCode`，错误文本用纯函数获取：

```cpp
#include <cstdio>

#include <origin/filesystem/filesystem.h>
#include <origin/utils/utils_error_code.h>

const ErrorCode rc = origin::filesystem::create_dir("logs/app");
if (rc != ERR_COMM_SUCCESS) {
    std::printf("failed: %s (code=0x%llx)\n",
                get_utils_err_msg(rc),
                static_cast<unsigned long long>(rc));
}
```

需要产出值的接口有两套用法（双 API）：

```cpp
// 1) ErrorCode 版：完整错误信息由返回值给出
std::string text;
if (origin::filesystem::read_text_file("app.conf", text) != ERR_COMM_SUCCESS) {
    // 处理错误
}

// 2) 值返回版：直接拿结果，失败时返回默认值（空串 / 0 / UNKNOWN 等）
const std::string text2 = origin::filesystem::read_text_file("app.conf");
```

同名重载按参数区分：值返回版没有出参，失败语义是"返回默认值"，无法区分具体错误原因；
需要区分时用 `ErrorCode` 版。格式化接口（`format_time_string` 等）同理，值返回版还保留
了 `format` / `timeZone` 的默认值。

注意：

- 错误码只通过返回值传递，库内不再保留 thread-local 错误状态；
- 错误码随返回值传递，异步/协程切换不会丢上下文；
- 需要区分"不存在/权限不足"时，直接比较返回码。

## 二、C++ 标准库的错误类别

### 2.1 三个内置类别

| 类别 | 错误码来源 | 跨平台性 | 典型场景 |
|---|---|---|---|
| `std::generic_category` | C 标准库（`<cerrno>` / `std::errc`） | 完全一致 | 参数无效、文件不存在等通用错误 |
| `std::system_category` | 操作系统（`errno` / `GetLastError()`） | 平台相关 | 系统调用、Win32 API 失败 |
| `std::iostream_category` | I/O 流（`std::io_errc`） | 标准一致 | 流状态错误 |

```cpp
#include <cerrno>
#include <system_error>

std::error_code ec = std::make_error_code(std::errc::no_such_file_or_directory);
if (ec == std::errc::no_such_file_or_directory) {
    // 跨平台比较：推荐用 std::errc 而不是具体 errno 值
}
```

### 2.2 `generic_category` 与 `system_category` 的区别

- `generic_category`：错误码在所有平台含义一致，适合做**跨平台判断**；
- `system_category`：错误码来自底层系统，适合在调用系统 API 后**原样上报**；
- Windows 上 `std::system_category()` 的错误码是 Win32 错误码（如
  `ERROR_FILE_NOT_FOUND` = 2），Unix 上通常是 `errno`。

比较错误时优先用 `std::errc`：

```cpp
if (ec == std::errc::permission_denied) { ... }   // 跨平台
```

### 2.3 与 Origin 错误码的关系

Origin 内部会把系统错误转换成自己的 `ErrorCode`（例如文件系统封装把
`std::error_code` / 异常转换成 `ERR_UTILS_*`），对外统一用 `ErrorCode` 返回值
传递（数据返回接口另提供同名的值返回重载）。因此：

- 使用 Origin 的工具接口时，直接读取/比较返回的 `ErrorCode`，不需要自己处理
  `std::error_code`；
- 直接调用标准库 / 系统 API 时，仍按标准方式使用 `std::error_code`。

### 2.4 实现边界（读代码前必看）

- `MAKE_ERROR` **不做字段掩码**。`severity/module/type/id` 超出 16 位会污染相邻字段；
  新增宏时先确认取值在范围内，必要时加 `static_assert`（见 COMMON-005）。
- `system_category` 到 Origin 错误码的映射很有限：Windows 只覆盖少量 Win32 错误，
  POSIX 基本统一坍缩成 `ERR_COMM_SYSTEM_ERROR`。不要指望从 `ErrorCode` 反推出精确的
  `errno`（见 UTILS-005）。
- **值返回重载不是 `noexcept`**：`get_directory`、`join_paths`、`to_absolute_path` 等
  路径辅助函数在非法 UTF-8 / 超长路径下仍可能抛异常（见 UTILS-007）。处理不可信输入时
  用 `ErrorCode` 版，或在边界统一兜底。
- C ABI 的目标是“返回 `ErrorCode`、不抛异常”，但当前尚未完全做到：非法枚举、
  `working_env`、debug logger 等入口仍可能抛（CROSS-001、LOGGING-004、COMMON-010）。
  C 侧调用要按“可能异常/可能没有错误码”做防御，不能假设失败一定返回错误码。
- 文案函数未命中会逐级回退：`get_utils_err_msg` / `get_container_err_msg` → Common 文案。
  错误文本只用于展示，**不要解析文本做逻辑判断**。

## 三、添加新错误码

1. 在对应模块的 `*_error_code.h` 里定义 `ErrorId` 枚举值；
2. 用模块宏构造宏：

```cpp
#define MAKE_MYMOD_ERROR(errId) \
    MAKE_ERROR(ERR_SEV_ERROR, MYMOD_MODULE_ID, MYMOD_ERR_TYPE_GENERAL, errId)

#define ERR_MYMOD_BAD_CONFIG MAKE_MYMOD_ERROR(MYMOD_ERR_ID_BAD_CONFIG)
```

3. 新模块的 `MODULE_ID` 选一个未占用的值（当前 1/2/3 已用）；
4. 如需可读文本，在模块内提供类似 `get_comm_err_msg` 的转换函数。

## 四、最佳实践

- **对外统一用 `ErrorCode`**，避免把 `std::error_code` 泄漏到公共接口；
- **不关心错误原因时可用值返回重载**（失败返回空串 / `0` / `UNKNOWN`）；
  需要区分错误原因时用 `ErrorCode` 版；
- **跨平台比较用 `std::errc`**，不要直接比较 Windows 的 Win32 错误码；
- **错误信息只用于展示**，逻辑判断用错误码本身；
- 在异步/线程池场景下，错误码随返回值传递，不需要额外保存上下文。
