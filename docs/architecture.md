# Origin 架构与系统模型

本文回答“Origin 由什么组成、边界在哪里、对象怎么活、构建出什么”，不重复各组件 API。
新会话建议按 **architecture → status → testing → 任务相关 guide → audit** 的顺序读。

## 1. 模块与构建产物

| 模块 | CMake 目标 | 类型 | 命名空间 | 公共头文件 |
|---|---|---|---|---|
| Common | `Origin::Common` | `SHARED` | `origin::common`；C ABI 在全局 | `src/libs/common/include/origin/common/` |
| Container | `Origin::Container` | `INTERFACE`（纯头文件） | `origin::container` | `src/libs/container/include/origin/container/` |
| Utils | `Origin::Utils` | `SHARED` | `origin::filesystem` / `date_time` / `strings` / `thread` / `process` | `src/libs/utils/include/origin/` |
| Logging | `Origin::Logging` | `SHARED` | `origin::logging`；C ABI 在全局 | `src/libs/logging/include/origin/logging/` |

要点：

- 实际输出名是 `OriginCommon` / `OriginUtils` / `OriginLogging`，**不是** `Origin::Common`；后者只是 CMake alias。
- Container 没有二进制，也没有导出宏；`BlockingQueue` / `ConcurrentBlockingQueue` 都是头文件实现。
- Debug 构建带 `d` 后缀（`OriginUtilsd`），安装导出仍使用 alias；版本 `2.0.0`，`SOVERSION` 为 `2`，没有静态库产物。

依赖方向固定，低层不能 include 高层：

```text
Common  <- Container
Common  <- Utils
Common / Container / Utils  <- Logging
```

`src/tests` 是聚合入口，不受分层限制。改 include 后跑 `./script/check_layers.sh`。

## 2. 错误模型

`ErrorCode` 是 `uint64_t`，按位分成四段（从高到低）：

```text
63-48 severity | 47-32 module | 31-16 type | 15-0 id
```

- `module`：`COMMON=1`、`UTILS=2`、`CONTAINER=3`；成功值是 `ERR_COMM_SUCCESS`（`0`）。
- 构造用 `MAKE_ERROR(severity, module, type, id)`，各模块再包一层 `MAKE_*_ERROR`；`MAKE_ERROR` 不做字段掩码，传超宽值会污染相邻字段。
- 文案函数是纯函数：`get_comm_err_msg`、`get_utils_err_msg`、`get_container_err_msg`，后两者未命中会回退到 Common。
- 可失败接口的标准形态是 `ErrorCode fn(..., Out& out)`，同名 inline 重载直接返回值；值版只适合“不关心失败原因”的场景。
- 部分路径辅助函数（`get_directory`、`join_paths`、`to_absolute_path` 等）的值返回重载仍可能抛异常，不能当 `noexcept`。

完整位布局、映射边界和新增错误码步骤见 [错误码指南](guide/system_error_code.md)。

## 3. 日志模型

对象关系：

```text
Registry (单例)
 ├─ __root_logger__ : Logger  ← 构造时即创建并注册
 ├─ 其他已注册 Logger
 └─ root TaskPool (可选)

Logger ──shared_ptr──> Sink ──unique_ptr──> Formatter
```

关键事实：

- **root logger 在注册表里**，`snapshot_loggers` / `set_level_all` / `set_pattern_all` 都会包含它；`remove_logger` / `remove_all` 可以把它删掉，之后自由函数会抛 `std::runtime_error`。
- `create_logger<T>(...)` 只 `make_shared`，**不注册**，也不继承全局级别/格式；`initialize_logger(logger)` 才套用全局设置并注册。
- formatter 只挂在 sink 上；`logger->set_pattern()` / `set_formatter()` 会 clone 后逐个下发给 sink，保留 formatter 对象再修改不会生效。
- 同步 logger 在调用线程直接写；异步 logger 只持 `weak_ptr<TaskPool>`，池失效后 `log` / `flush` 静默降级为同步，队列满时阻塞调用线程。
- `flush_all()` 只入队，不等待；`shutdown()` 排空队列并 join，但**不主动 flush sink**，也不保证之后还能继续写。
- C ABI 用不透明句柄（`LoggerSt*` / `SinkSt*` / `FormatterSt*` / `TaskPoolSt*`），创建/销毁成对；`origin_logger_name` 返回借用指针。

创建/注册流程、sink 行为和已知缺陷见 [日志指南](guide/logging.md)。

## 4. 所有权与生命周期

| 对象 | 谁拥有 | 释放方式 | 注意 |
|---|---|---|---|
| Logger | 调用方 `shared_ptr`；注册表另持一份 | `shared_ptr` 归零 | `remove_logger` 只移除注册表引用 |
| Sink | Logger 按 `shared_ptr` 持有 | 随最后一个 Logger 释放 | 多个 logger 可共享同一个 sink |
| Formatter | Sink 按 `unique_ptr` 持有 | 随 Sink 释放 | 设置时是 clone，不是转移原对象 |
| TaskPool | 调用方 / Registry 持有 | `shutdown()` + 释放 | AsyncLogger 只持 `weak_ptr` |
| C 句柄 | 调用方 | `origin_destroy_*` | 必须成对，顺序不敏感 |
| 队列元素 `T`（`optional` 槽位） | 队列 | 出队 / `clear` 时 `reset` | 不再要求 `T` 默认构造 |

C ABI 的通用约定：跨边界只用 C 类型、UTF-8、显式所有权；异常不能穿出 `extern "C"`。当前实现尚未完全做到（CROSS-001、LOGGING-004、COMMON-010 等仍 open），调用 C 接口时要按“可能失败/可能不返回错误”设计防御。

## 5. 线程安全速查

| 对象 | 是否线程安全 | 说明 |
|---|---|---|
| `BlockingQueue` | **否** | 无锁、无原子；只适合单线程或外部加锁 |
| `ConcurrentBlockingQueue` | 是 | 内部 mutex + 条件变量；`dequeue` 用阻塞锁并通知生产者，`at/operator[]` 持锁返回元素拷贝（CONTAINER-006 已关闭） |
| Logger 级别 / flush 级别 | 是 | `std::atomic` |
| Sink 写入 | 基本是 | 内部互斥锁；失败可观测性仍不完整 |
| Registry | 是 | `std::mutex`；key 是非拥有 `string_view`，同名替换存在悬垂风险 |
| Async 队列 | 是 | `ConcurrentBlockingQueue`；满时阻塞 |

并发相关改动必须补失败路径或并发测试，不要假设“容器/日志已经线程安全”。

## 6. 构建与集成形态

- CMake 最低 3.14；`CMakePresets.json` v9 需要 CMake 3.30+。
- 选项：`ORIGIN_BUILD_TESTS`（OFF）、`ORIGIN_ENABLE_ASAN`（OFF）、`ORIGIN_INSTALL`（ON）、`ORIGIN_DEBUG_LOG_ALL_LEVELS`（OFF）。
- 所有 `*_build` preset 继承 `project_build_base`，targets 是 `all` + `install`；**`cmake --build --preset=...` 会顺带安装**。
- 安装导出 `lib/cmake/Origin/OriginConfig.cmake`，兼容策略 `SameMinorVersion`；下游用 `find_package(Origin CONFIG)` 或 `FetchContent`。
- CPack 生成器是 `ZIP;TGZ`；`*_pkg` preset 的 `cpack --preset` 按 preset 配置产出。
- 版本唯一来源是根 `CMakeLists.txt` 的 `ORIGIN_VERSION_FILE`；非发布构建的完整串是 `2.0.0-dev+g<sha>[-dirty]`，只进 `build_info.h`，`version.h` 保持裸 semver。

## 7. 平台差异入口

- 文件系统封装对外 UTF-8，Windows 内部转 UTF-16；但 `process::get_proc_path` 当前用 `GetModuleFileNameA`（ANSI），中文路径会丢。
- 文件读写走文本模式：Windows 写 `\n` 会变 CRLF；`read_binary_file` 目前也以文本模式打开，遇到 `0x1A` 可能提前 EOF。
- 时间转换只有本地时区版；`date_time_to_timestamp` 用 `mktime` 且 `tm_isdst=0`，跨 DST 可能偏 1 小时；`set_curr_thread_name` 只写 `thread_local`，不调用 OS。

更多平台行为和错误映射见 [文件系统指南](guide/filesystem.md) 与 [错误码指南](guide/system_error_code.md)。

## 8. 术语

- **target / preset**：CMake 目标与 `CMakePresets.json` 中的配置；`*_build` 自带 install。
- **root logger**：名字 `__root_logger__` 的全局 logger，自由函数默认写它。
- **value overload**：与 `ErrorCode` 版同名、直接返回值的重载。
- **C ABI**：`extern "C"` 导出层，供 C#/Python 等封装；不暴露 STL。
- **finding**：`docs/audit/findings.json` 中的一条审计记录，状态以该文件为准。

相关文档：[状态快照](status.md) · [测试与验证](testing.md) · [日志](guide/logging.md) · [文件系统](guide/filesystem.md) · [错误码](guide/system_error_code.md) · [集成](integration.md)
