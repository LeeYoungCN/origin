# Logging 模块使用说明

## 1. 概述

`Logging` 是 `origin` 的日志组件，源码在 `src/libs/logging`，依赖 `Common`、`Container`、`Utils` 三个组件。

| 项 | 值 |
|---|---|
| CMake 目标名 | `Logging` |
| 产物名 | `originLogging`（Debug 构建带 `d` 后缀，如 `originLoggingd.dll`） |
| 语言标准 | C++20（消息格式化使用 `std::format`） |
| 对外头文件 | `#include "origin/logging/logging.hpp"` |
| C 接口 | `#include "origin/logging/c/logging_c.h"` |

模块分四层：

```
日志调用 (Logger::info/...)  →  Logger（过滤级别、组装 LogMsg）  →  Sink（落地：控制台/文件）  →  Formatter（拼格式）
                                        ↑
                                    Registry（全局 root logger、全局级别/格式、注册表）
```

静态库形态可用 `LOGGING_STATIC` 宏控制（默认导出动态库）。

## 2. 快速开始

库内部已经默认创建了一个 root logger（`SyncLogger` + `StdoutSink`），所以什么都不配置也能直接输出：

```cpp
#include "origin/logging/logging.hpp"

using namespace origin::logging;

int main()
{
    set_level(LogLevel::DEBUG);
    set_pattern("[%d][%L][%s:%#] %v");

    info("service started, version={}", "1.0.0");
    debug("worker {} started", 3);
    warn("config missing, use default {}", 10);
    error("request failed: {}", 404);
    flush();

    return 0;
}
```

示例里的 `set_level(LogLevel::DEBUG)` 一步就能看到 `debug` 日志 —— sink 默认是 `TRACE`，不会额外过滤；只有需要按 sink 分流时才去设 sink 级别（见第 4 节）。

CMake 里链接组件即可（头文件路径由组件自动带出）：

```cmake
target_link_libraries(your_target PRIVATE Logging)
```

## 3. 核心概念

| 概念 | 类型 | 作用 |
|---|---|---|
| `LogLevel` | `enum class` | 级别：`TRACE`/`DEBUG`/`INFO`/`WARN`/`ERR`/`FATAL`/`OFF` |
| `Logger` | 抽象基类 | 级别过滤、组装 `LogMsg`、分发到 sink；实现有 `SyncLogger`、`AsyncLogger` |
| `Sink` | 抽象基类 | 日志落地点；实现有 `StdoutSink`、`StderrSink`、`BasicFileSink`、`DailyFileSink`、`RotatingFileSink` |
| `Formatter` | 抽象基类 | 把 `LogMsg` 拼成文本；实现有 `PatternFormatter` |
| `LogMsg` | `struct` | 一条日志的数据：logger 名、级别、内容、时间戳、线程/进程 id、源位置 |
| `LogSource` | `struct` | 源位置：`file`、`line`、`func` |
| `Registry` | 单例（内部） | root logger、全局级别/格式、logger 注册表、全局线程池 |

`Logger` 与 `Sink` 都是一对多：一个 logger 可以挂多个 sink，任一 sink 都可以独立设置级别和格式。

## 4. 日志级别与过滤

| 枚举 | 数值 | 缩写 | 全名 |
|---|---|---|---|
| `LogLevel::TRACE` | 0 | `T` | `TRACE` |
| `LogLevel::DEBUG` | 1 | `D` | `DEBUG` |
| `LogLevel::INFO` | 2 | `I` | `INFO` |
| `LogLevel::WARN` | 3 | `W` | `WARN` |
| `LogLevel::ERR` | 4 | `E` | `ERROR` |
| `LogLevel::FATAL` | 5 | `F` | `FATAL` |
| `LogLevel::OFF` | 6 | `O` | `OFF` |

过滤规则是**两级过滤**，两条都满足才会真正落盘：

1. logger 级别：`level >= logger.level()` 才继续（`OFF` 永远不输出）；
2. sink 级别：`level >= sink.level()` 才写这个 sink。

但两级的默认值不同，这一点和 spdlog 一致：

| 对象 | 默认级别 | 含义 |
|---|---|---|
| logger | `INFO` | 默认只放 info 及以上，这是唯一的默认过滤层 |
| sink | `TRACE` | 默认**不过滤**，logger 放过来的都写 |

所以平时只调 logger 就够了：

```cpp
set_level(LogLevel::DEBUG);   // 一步到位，sink 默认不挡
```

只有需要**按 sink 分流**时才去动 sink 级别（比如控制台只留 info+、文件要 debug+）：

```cpp
auto console = create_sink<StdoutSink>();
console->set_level(LogLevel::INFO);

auto file = create_sink<BasicFileSink>("logs/app.log", true);
file->set_level(LogLevel::DEBUG);

auto logger = create_logger<SyncLogger>("app", {console, file});
logger->set_level(LogLevel::DEBUG);   // logger 这层放到 debug
```

`sink->set_level(LogLevel::OFF)` 等价于**关掉这个 sink**：`OFF` 比任何级别都大，`level >= OFF` 永不成立。

`should_log(level)` 可以在格式化代价高的日志前先判断，避免无谓的格式化开销：

```cpp
if (should_log(LogLevel::DEBUG)) {
    debug("dump: {}", build_expensive_string());
}
```

`flush_on(level)` 表示"级别达到该值就立即 flush"。默认是 `OFF`（不自动 flush），关键日志建议：

```cpp
flush_on(LogLevel::ERR);
```

## 5. 快捷日志函数（root logger）

`logging.hpp` 为每个级别提供了一组转发到 root logger 的模板函数，四种常用形态：

```cpp
// 1. 格式化 + 无源位置
info("user {} logged in", 42);

// 2. 格式化 + 带源位置（文件名/行号/函数）
info(LOG_SRC_LOCAL, "user {} logged in", 42);

// 3. 已经拼好的字符串/可转字符串的对象
info(std::string("already formatted"));

// 4. 任意级别
log(LOG_SRC_LOCAL, LogLevel::WARN, "value={}", 1);
```

| 函数 | 说明 |
|---|---|
| `trace/debug/info/warn/error/fatal(...)` | 转发到 root logger 的对应级别 |
| `log(level, ...)` / `log(source, level, ...)` | 指定级别 |
| `set_level` / `level` / `should_log` | root logger 的级别读写与判断 |
| `flush_on` / `flush_level` / `should_flush` | flush 策略 |
| `set_pattern` / `set_formatter` | root logger 的格式 |
| `flush()` | 立即 flush |

`LOG_SRC_LOCAL` 展开成 `{__FILE__, __LINE__, __FUNCTION__}`；不想带源位置就用不带 `source` 的重载（等价于 `LOG_SRC_EMPTY`）。

## 6. 创建 Logger

```cpp
#include "origin/logging/logging.hpp"

using namespace origin::logging;

// 单 sink
auto fileSink = create_sink<BasicFileSink>("logs/app.log", true);
auto logger = create_logger<SyncLogger>("app", fileSink);

// 多 sink（initializer_list 或 vector 都可以）
auto outSink = create_sink<StdoutSink>();
auto multi = create_logger<SyncLogger>("app", {outSink, fileSink});

// 挂到 root logger 上，之后 info(...) 就都走这个 logger
set_root_logger(logger);
```

约束与行为：

| 项 | 说明 |
|---|---|
| logger 名 | 不能为空，否则构造抛 `std::invalid_argument` |
| sink | 不能为空、不能是 `nullptr`，否则抛 `std::invalid_argument` |
| 重名 | `register_logger` 返回 `false`；`register_or_replace_logger` 覆盖；`set_root_logger` 走覆盖逻辑 |
| 默认级别 | 新建 logger 是 `INFO`，与全局默认一致 |
| sink 共享 | logger 按 `shared_ptr` 持有 sink，sink 生命周期跟随 logger |

`create_logger<SyncLogger>(name, sink)` 内部走 `std::make_shared`，返回 `std::shared_ptr<SyncLogger>`（模板实参决定静态类型）。

## 7. Sink 一览

| Sink | 构造参数 | 落地点 |
|---|---|---|
| `StdoutSink` | 无 | 标准输出 |
| `StderrSink` | 无 | 标准错误 |
| `BasicFileSink` | `(file, overwrite = true)` | 单个文件，从头写或追加 |
| `DailyFileSink` | `(file, hour = 0, minute = 0, maxFiles = 30, overwrite = false)` | 按天滚动 |
| `RotatingFileSink` | `(file, maxFileSize = 10MB, maxFiles = 100, rotateOnOpen = false)` | 按大小滚动 |

文件命名规则（`file` 的扩展名会保留）：

| Sink | 实际文件 |
|---|---|
| `BasicFileSink("logs/app.log")` | `logs/app.log` |
| `RotatingFileSink("logs/app.log")` | 当前写 `logs/app.log`，滚动出的历史文件是 `logs/app.1.log`、`logs/app.2.log`……（索引 1~20000，超过后回绕，超出 `maxFiles` 的最旧文件被删除） |
| `DailyFileSink("logs/app.log")` | 当前写 `logs/app.log`，轮转出的历史文件是 `logs/app.20261108.log`（年月日），超出 `maxFiles` 的最旧文件被删除 |

其他要注意的点：

- 不传路径时用默认值：`<可执行文件目录>/logs/<可执行文件名>.<suffix>`，例如 `E:\app\app.exe` → `E:\app\logs\app.log`；
- 构造时就会打开文件，打开失败抛 `std::runtime_error`，目录不存在也算失败；
- `overwrite` 参数（`BasicFileSink` / `DailyFileSink`）：`true` 表示启动时清空，`false` 表示追加；
- `RotatingFileSink` 没有 `overwrite`，`rotateOnOpen = true` 表示启动时先把已存在的文件转成历史文件（变成 `.1`），再新开一个空文件；
- `DailyFileSink` 的 `hour`/`minute` 是每天的滚动时刻（默认 00:00），取值越界抛 `std::out_of_range`；
- `get_file_list()` 可以拿到已经滚动出来的文件列表，便于排查或打包；
- 每个 sink 有自己的级别和格式，`logger->set_pattern()` 会把格式**下发到它所有的 sink**；
- sink 级别默认 `TRACE`（不过滤），不调就永远不挡消息，需要按 sink 分流时才显式 `set_level`。

## 8. 异步日志与线程池

同步 logger 在调用线程里直接写文件，高并发场景会阻塞业务线程。异步 logger 把写操作投递到线程池：

```cpp
// 1. 创建并注册全局线程池（capacity 为队列容量，threadCnt 为工作线程数）
auto pool = create_task_pool(8192, 2);
set_root_task_pool(pool);

// 2. 之后创建的 AsyncLogger 自动使用全局池
auto sink = create_sink<BasicFileSink>("logs/app.log", true);
auto logger = create_logger<AsyncLogger>("app", sink);
```

也可以显式指定池：

```cpp
auto logger = create_async_logger("app", sink, pool);
```

注意事项：

| 项 | 说明 |
|---|---|
| 没有池 | `AsyncLogger` 构造抛 `std::runtime_error`（"requires a task pool to be registered in the registry"） |
| 池只能设一次 | `set_root_task_pool` 已存在时返回 `false`，不会覆盖 |
| 参数校验 | `capacity` 或 `threadCnt` 为 0 时构造抛 `std::invalid_argument` |
| 队列满 | 投递用 `enqueue_wait`，队列满会**阻塞调用线程**（背压）；不丢日志，但会拖慢业务线程 |
| 生命周期 | `AsyncLogger` 内部持有 `weak_ptr<TaskPool>`，池必须比 logger 活得久 |
| 退出前 | `flush_all()` 只是往队列投递 flush 任务，**不等待**；真正保证落盘的是线程池销毁或 `shutdown()`（会 join 工作线程、把队列排空） |

## 9. 日志格式（PatternFormatter）

默认格式：`[%d][%l][%s:%#]: %v`

| 占位符 | 内容 | 示例 |
|---|---|---|
| `%d` | 日期时间（固定 `YYYY-MM-DD HH:MM:SS.mmm`） | `2026-11-08 20:15:30.123` |
| `%n` | logger 名 | `app` |
| `%l` | 级别缩写 | `I` |
| `%L` | 级别全名 | `INFO` |
| `%s` | 源文件名 | `main.cpp` |
| `%g` | 源文件完整路径 | `E:\code\app\main.cpp` |
| `%#` | 行号 | `42` |
| `%!` | 函数名 | `main` |
| `%t` | 线程 id | `12345` |
| `%P` | 进程 id | `6789` |
| `%v` | 日志内容 | `service started` |
| `%%` | 百分号 | `%` |

规则细节：

- 未知占位符按原样输出（如 `%x` → `%x`），不会报错；
- 空 pattern 抛 `std::invalid_argument`；
- 源位置相关占位符在没传 `source` 时会输出占位文本：`%s` → `Filename`、`%g` → `FilePath`、`%!` → `Function`。

常用写法：

```cpp
set_pattern("[%d][%L][%s:%#][%t] %v");       // root logger 的格式
set_pattern_all("[%d][%L] %v");              // 全局格式：同步到所有已注册 logger，并作为新 logger 的默认
logger->set_pattern("%v");                   // 单个 logger（下发到它的 sink）
logger->set_formatter(create_formatter<PatternFormatter>("[%L] %v"));
```

## 10. 全局管理与多模块

| 函数 | 作用 |
|---|---|
| `initialize_logger(logger)` | 套用全局格式、全局级别、全局 flush 级别，并注册（`Registry::initialize_logger` 默认 `autoRegister = true`） |
| `set_level_all(level)` | 设置全局级别并同步到所有已注册 logger |
| `flush_on_all(level)` | 同上，作用于 flush 策略 |
| `set_pattern_all(pattern)` | 设置全局格式并同步到所有已注册 logger |
| `set_formatter_all(formatter)` | 同上，自定义 Formatter 版本 |
| `flush_all()` | flush 所有 logger 的所有 sink |
| `apply_all(fun)` | 对每个已注册 logger 执行 `fun`（先取快照再回调，回调里可安全调用注册表接口） |
| `log_all(source, level, ...)` | 向所有已注册 logger 各写一条日志，返回通过各 logger 级别过滤的数量（sink 级别仍可能再丢弃） |
| `shutdown()` | 清空注册表（含 root logger）并释放全局线程池 |

`log_all` 的三种调用形态与各自的落点：

```cpp
log_all(LOG_SRC_LOCAL, LogLevel::INFO, "config reloaded, version={}", 2);  // 格式化：消息只格式化一次
log_all(LOG_SRC_LOCAL, LogLevel::INFO, std::string("already formatted"));  // 已拼好的字符串
log_all(LOG_SRC_LOCAL, LogLevel::INFO, "{plain}");                         // 字面量：按消息处理，不当格式串
```

注意两点：

- 消息只格式化一次，再分发给每个 logger，不会 N 个 logger 格式化 N 遍；
- 若多个 logger 共享同一个 sink，这个 sink 会收到多条（每个 logger 各一条，`loggerName` 不同）。

注册表相关：

```cpp
register_logger(logger);            // 重名返回 false
register_or_replace_logger(logger); // 覆盖同名
get_logger("app");                  // 不存在返回 nullptr
is_logger_exists("app");
remove_logger("app");               // 删的是 root logger 时，root 同时被置空
remove_all();                       // 清空全部，root 也没了
```

root logger 的名字是 `__root_logger__`（`LOGGING_ROOT_LOGGER_NAME`）。`remove_all()` 之后 `set_level()`、`set_pattern()` 等会抛 `std::runtime_error`，需要注意调用顺序。

## 11. 扩展：自定义 Sink 与 Formatter

自定义 Formatter，只需实现格式化与克隆：

```cpp
class JsonFormatter : public origin::logging::Formatter
{
public:
    void format(const origin::logging::LogMsg &msg, std::string &content) override
    {
        content += std::format(R"({{"level":"{}","msg":"{}"}})",
                               origin::logging::log_level_full_name(msg.level),
                               msg.data);
    }

    std::unique_ptr<origin::logging::Formatter> clone() const override
    {
        return std::make_unique<JsonFormatter>(*this);
    }
};
```

| 自定义 Sink | 直接实现 `Sink` 的纯虚函数即可（模块内部的 `SinkBase`/`SinkImplBase` 是 pImpl 封装，内部头文件不对外）。纯虚函数如下： |

| 纯虚函数 | 说明 |
|---|---|
| `log(const LogMsg&)` | 写一条日志 |
| `flush()` | 刷盘 |
| `set_level` / `level` / `should_log` | 级别过滤 |
| `set_pattern` / `set_formatter` | 格式 |
| `param_string()` | 返回描述字符串（用于日志/调试） |

## 12. C 接口

`logging_c.h` 提供与 C++ 版一一对应的 C API，日志格式化走 `printf` 风格：

```c
#include "origin/logging/c/logging_c.h"

SinkSt *sink = origin_create_stdout_sink();
SinkSt const *sinks[1] = {sink};
LoggerSt *logger = origin_create_sync_logger("c_app", sinks, 1);

origin_logger_set_level(logger, LOG_LEVEL_C_DEBUG);
origin_sink_set_level(sink, LOG_LEVEL_C_TRACE);
origin_logging_set_root_logger(logger);

ORIGIN_LOGGING_INFO("hello from C, value=%d", 1);
ORIGIN_LOGGER_WARN(logger, "level=%s", origin_log_level_full_name(LOG_LEVEL_C_WARN));

origin_logging_flush();
origin_destroy_logger(logger);
origin_destroy_sink(sink);
```

| 宏 | 说明 |
|---|---|
| `ORIGIN_LOGGING_TRACE/DEBUG/INFO/WARN/ERROR/FATAL(fmt, ...)` | 写 root logger，自动带 `__FILE__`/`__LINE__`/`__FUNCTION__` |
| `ORIGIN_LOGGING_LOG(level, fmt, ...)` | 指定级别写 root logger |
| `ORIGIN_LOGGER_*` / `ORIGIN_LOGGER_LOG(logger, level, ...)` | 写指定 logger |

注意点：

- C 版是 `printf` 风格（`%d`/`%s`），C++ 版是 `std::format` 风格（`{}`），不要混用；
- C 版对象都要用 `origin_destroy_*` 释放；`LoggerSt`/`SinkSt` 内部都是 `shared_ptr`，logger 持有 sink 的引用，
  销毁顺序不敏感，但两个都要销毁，否则泄漏；
- `origin_create_async_logger` 需要传入 `TaskPoolSt*`；用 `origin_create_async_logger_use_root_tp` 可以复用全局池；
- 版本查询：`origin_logging_version()`、`origin_logging_build_time()`。

## 13. 线程安全与生命周期

| 项 | 说明 |
|---|---|
| logger 级别 / flush 级别 | `std::atomic`，多线程读写安全 |
| sink 写入 | 内部互斥锁保护，多个线程共用一个 logger 是安全的 |
| 注册表 | 内部 `std::mutex`，注册/查询可并发 |
| 异步写入 | 投递到 `TaskPool`（`ConcurrentBlockingQueue`），队列满时阻塞调用线程，不会静默丢日志 |
| 退出 | 异步场景先 `flush_all()`，再让线程池销毁或 `shutdown()`（join 工作线程、排空队列）；之后不要继续使用 root logger 相关接口 |

C++ 侧版本信息：

```cpp
#include "origin/logging/version.h"

constexpr auto v = origin::logging::compiled_version();     // 编译期版本
std::string_view runtime = origin::logging::version();      // 运行期实际链接到的库版本
std::string_view built = origin::logging::build_time();     // 运行期构建时间
```

## 14. 常见问题

| 现象 | 原因 | 处理 |
|---|---|---|
| 设了 `TRACE`/`DEBUG` 还是没输出 | logger 级别没调，或某个 sink 被显式调高了 | 检查 `logger->level()` 和各 `sink->level()` |
| 异步日志偶发丢失 | 进程退出时队列还没排空 | 退出前 `flush_all()` 并销毁线程池 / 调用 `shutdown()` |
| `create_logger<AsyncLogger>` 抛 `runtime_error` | 没有注册全局线程池 | 先 `set_root_task_pool(create_task_pool(...))` |
| 重名 logger 注册失败 | `register_logger` 遇到同名返回 `false` | 用 `register_or_replace_logger`，或先 `remove_logger` |
| 文件 sink 构造抛 `runtime_error` | 目录不存在 / 文件被占用 | 先建目录；确认没有别的进程独占 |
| `%s`/`%#` 输出是 `Filename`/`0` | 调用时没带源位置 | 需要位置信息就用 `LOG_SRC_LOCAL` 的那个重载 |
| `shutdown()` 后调用 `set_level` 抛异常 | root logger 已被清空 | 重新 `set_root_logger`，或调整调用顺序 |
| 中文在控制台乱码 | 控制台代码页与 UTF-8 输出不一致 | `chcp 65001`，或改用文件 sink 查看 |
| 新 logger 没继承全局格式 | 全局格式只对已注册 logger 生效 | 创建后调用 `initialize_logger(logger)`，或直接 `set_pattern_all` |

## 15. 相关文档

- 日期时间工具（`%d` 依赖的格式化能力）：`doc/DateTimeUtils_UsageGuide.md`
- 文件工具（文件 sink 的读写基础）：`doc/FilesystemUtils.md`
