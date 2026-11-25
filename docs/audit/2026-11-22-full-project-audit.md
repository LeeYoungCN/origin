# 2026-11-22 全项目审计

## 元信息

- 仓库：`origin`
- 基线：`c37a364`（P0 修复提交）；审计时工作区干净
- 范围：`src/libs/common`、`src/libs/container`、`src/libs/logging`、`src/libs/utils`，以及 C API、CMake 和测试覆盖
- 方法：模块清点 + 静态阅读 + 高风险模式扫描 + 现有测试对照
- 限制：
  - 不是形式化证明，未做全量 ASan/TSan 运行；
  - `logging` / `utils` 的详细证据见 [2026-11-22-logging-utils-audit.md](2026-11-22-logging-utils-audit.md)；
  - 行号基于上述基线，后续修改可能偏移。

## 结论摘要

| 模块 | 新增问题 | 说明 |
|---|---:|---|
| `common` | 12 | C API 异常边界、平台路径、宏定义、格式化 |
| `container` | 11 | 对象生命周期、并发锁、CString 溢出 |
| `logging` / `utils` | 0 新增 | 沿用既有 28 项审计，状态已更新 |
| 跨模块 | 3 | 测试覆盖、C ABI 策略、构建宏 |

全项目问题总数：**54**

- 既有 28 项：21 `open`、2 `partial`、5 `fixed`
- 新增 26 项：26 `open`
- 合计：**47 `open`、2 `partial`、5 `fixed`**

## 新增 P0

### COMMON-001 Linux `readlink` 失败时越界写

- 状态：`open`
- 位置：`src/libs/common/src/debug/working_env.cpp:95-96`
- 证据：

  ```cpp
  auto length = readlink("/proc/self/exe", path, BUFFER_LEN - 1);
  path[length] = '\0';
  ```

  `readlink` 失败返回 `-1`，随后写 `path[-1]`。
- 影响：Linux 上执行 `origin_show_working_env()` 可能写坏栈。
- 建议：检查 `length < 0`；检查截断；失败时返回空路径并报告。
- 验证：模拟 `readlink` 失败/路径过长。

### CONTAINER-001 `BlockingQueue` 在已构造对象上 placement new

- 状态：`open`
- 位置：`src/libs/container/include/origin/container/blocking_queue.hpp:26-29`、`:46-49`、`:56-59`
- 证据：

  ```cpp
  std::vector<T> _items;          // 构造时默认构造 _maxItems 个 T
  T* elem_ptr = &_items[_tail];
  traits_type::construct(_alloc, elem_ptr, ...);  // 对活对象再次构造
  ```

- 影响：非平凡类型会绕过析构、覆盖活对象，属于未定义行为；`LogTask` 等带 `std::string` 的类型风险最高。
- 建议：用未初始化存储（`std::aligned_storage`/`std::byte` 缓冲 + 手动生命周期），或直接用 `std::deque`/环形缓冲并在覆盖前 `destroy`。
- 验证：用带计数器的非平凡类型测试构造/析构次数；ASan/MSan。

### CONTAINER-002 `ConcurrentBlockingQueue::dequeue` 未检查 try_lock

- 状态：`open`
- 位置：`src/libs/container/include/origin/container/concurrent_blocking_queue.hpp:80-89`
- 证据：

  ```cpp
  std::unique_lock<std::mutex> lock(_queueMtx, std::try_to_lock);
  if (_queue.empty()) { return false; }   // 没有 lock.owns_lock()
  ```

- 影响：抢锁失败时仍读写 `_queue`，与生产者并发，数据竞争。
- 建议：先检查 `lock.owns_lock()`；失败直接返回 false。
- 验证：多线程 producer/consumer 压力测试；TSan。

### CONTAINER-003 `CString::append(std::format...)` 截断后长度越界

- 状态：`open`
- 位置：`src/libs/container/include/origin/container/cstring.hpp:46-53`
- 证据：

  ```cpp
  auto rst = std::format_to_n(m_cstr + m_length, m_capacity - m_length - 1, ...);
  *rst.out = '\0';
  m_length += rst.size;   // rst.size 是完整输出长度，可能远大于可写空间
  ```

- 影响：`m_length` 可超过 `m_capacity`；下一次 append 计算 `m_capacity - m_length - 1` 发生无符号下溢，随后越界写。
- 建议：检查 `rst.size >= available`，截断时设置 overflow 错误并只增加实际写入长度；或直接使用 `std::string`。
- 验证：容量 8，追加 100 字符，再追加一次；ASan。

## 新增 P1

| ID | 位置 | 问题 | 建议 |
|---|---|---|---|
| COMMON-002 | `working_env.cpp:204-216` | `origin_show_working_env` 的 `current_path()` / iostream 可能抛异常，穿过 C ABI | C 入口统一 `try/catch`，失败返回错误码/空输出 |
| CONTAINER-004 | `cstring.hpp:68-87` | `vsnprintf` 返回负数被强转 `size_t`；容量计算可能下溢 | 检查 `len < 0`、可用空间、加法溢出 |
| CONTAINER-005 | `concurrent_blocking_queue.hpp:212-226` | `dequeue_try` 调用不存在的 `_queue.pop_front()`，一旦实例化即编译失败；即使存在也会重复移除 | 删除该调用，只保留 `_queue.dequeue(item)` |
| CONTAINER-006 | `concurrent_blocking_queue.hpp:257-269` | `operator[]`/`at` 解锁后返回元素引用，其他线程可并发修改 | 返回值拷贝，或持有锁的回调式 API |
| CONTAINER-007 | `concurrent_blocking_queue.hpp:80-92` | `dequeue`/`dequeue_try` 出队后不通知 `_pushCv`，等待中的生产者可能永久阻塞 | 出队后 `_pushCv.notify_one()` |
| COMMON-010 | `debug_logger.cpp:237-252` | `origin_debug_logger_c` 经 `va_list_to_string` 可能抛 `runtime_error`，未捕获 | C 入口 `try/catch` 或改为不抛的格式化 |

## 新增 P2

| ID | 位置 | 问题 | 建议 |
|---|---|---|---|
| COMMON-003 | `working_env.cpp:88-103` | `GetProcessPath` 固定 256 字节；Windows 忽略返回值/截断；macOS 忽略 `_NSGetExecutablePath` 失败 | 动态扩容；检查返回值；失败返回空 |
| COMMON-004 | `common_error_code.h:45-75` | `ERR_COMM_PARAM_OUT_OF_RANGE` 没有对应 message；`"Unkown error"` 拼写错误 | 补 case；修文案 |
| COMMON-005 | `error_code_types.h:38-41` | `MAKE_ERROR` 不对 module/type/id 做掩码，超宽值会溢出到相邻字段 | 各字段先 `& 0xFFFF`，或加 static_assert |
| COMMON-006 | `macros/attributes.h:4-56` | `ATTR_ALIGNED` 在 MSVC 使用 `__attribute__`；`ATTR_USED`/`ATTR_PURE`/`ATTR_CONST`/`ATTR_WEAK` 的 MSVC 替代不等价 | 按编译器分别实现，或删除未使用宏 |
| COMMON-007 | `macros/feature.h:6-45` | MinGW 未计入 `COMPILER_GCC`/`COMPILER_CLANG`，`SUPPORTS_THREAD_LOCAL=0`，随后把关键字 `thread_local` 重定义为 `__thread` | 把 `COMPILER_MINGW` 纳入；不要重定义关键字 |
| COMMON-008 | `macros/warnings.h:7-19` | `#pragma warning(push,4)`/GCC diagnostic push 没有对应的 pop；`SUPPRESS_LOSSY_CONVERSION_WARNING` 泄漏状态 | 提供 push/pop 宏，或删除全局 pragma |
| COMMON-009 | `types/process_types.h`、`types/thread_types.h` | 公共头直接包含 `<windows.h>`，宏污染严重，且 `ProcessId`/`ThreadId` 放在全局命名空间 | 用前置声明/窄类型；放进 `origin` 命名空间 |
| COMMON-011 | `utils/src/strings.cpp:20-35` | `format == nullptr` 未检查；第二次 `vsnprintf` 返回值未检查；`std::string(len)` 后写 `len+1` 字节 | 判空；用 `len+1` 缓冲或 `resize` 后回退 |
| COMMON-012 | `types/type_traits.h:22-23` | 使用 C++20 `std::remove_cvref_t`，注释却写 C++17；数字类型覆盖不全 | 改为 `remove_cv_t<remove_reference_t<T>>` 或更新文档 |
| CONTAINER-008 | `concurrent_blocking_queue.hpp:271-275` | `clear()` 不通知等待中的生产者/消费者；元素也未显式销毁 | `notify_all()`；按存储模型销毁元素 |
| CONTAINER-009 | `blocking_queue.hpp:26-36` | 要求 `T` 默认构造；`std::vector<T>(_maxItems)` 先构造全部元素 | 使用未初始化存储，或文档限制 T |
| CONTAINER-010 | `cstring.hpp:14-65` | `CString` 在全局命名空间；`length/capacity/last_error` 非 const；`std::forward(args)...` 没有按 `Args` 转发；缺 `<utility>` | 放入 `origin::container`；修 const/转发 |
| CONTAINER-011 | `container/tests` | 测试只用 `int` 和正常长度，未覆盖非平凡 T、溢出、`dequeue_try`、clear 唤醒 | 补生命周期/溢出/并发失败测试 |

## 跨模块

### CROSS-001 C ABI 异常策略不统一

- 状态：`open`
- 位置：`common/src/debug/working_env.cpp`、`common/src/debug/debug_logger.cpp`、`logging/src/c/*`
- 问题：部分 C 入口捕获异常，部分直接调用可能抛异常的 C++ 实现。
- 建议：所有 `extern "C"` 入口统一 `try/catch (...)`，错误通过返回值/出参报告；日志库自身失败不能抛。

### CROSS-002 测试缺少失败注入

- 状态：`open`
- 问题：`utils`/`logging` 已有部分错误路径测试，但 `common`/`container` 没有故障注入；容器并发测试主要覆盖正常路径。
- 建议：增加非平凡类型生命周期、容量边界、锁竞争、readlink 失败、C API 异常测试。

### CROSS-003 构建宏与平台检测分散

- 状态：`open`
- 问题：`ORIGIN_BUILD_*` 在 CMake，`COMPILER_*`/`OS_*` 在头文件，MinGW 在 `COMPILER_MINGW` 与 `COMPILER_GCC` 之间不一致。
- 建议：统一一套平台/编译器/构建类型宏，并在公共头中只使用这套宏。

## 既有审计

`utils` / `logging` 的 28 项详见：

- [2026-11-22-logging-utils-audit.md](2026-11-22-logging-utils-audit.md)
- `findings.json` 中 `UTILS-*`、`LOGGING-*`

当前状态：21 `open`、2 `partial`、5 `fixed`。

## 建议优先级

1. **P0**：`COMMON-001`、`CONTAINER-001`、`CONTAINER-002`、`CONTAINER-003`；
2. **P1**：`COMMON-002`、`CONTAINER-004`～`007`、`COMMON-010`；
3. **P2**：其余宏/类型/测试问题；
4. 修复任何容器问题时，先补非平凡类型和并发测试，再改实现。

## 验证记录

本次只做静态扫描和现有测试对照，没有新增代码/测试。现有测试基线：

```text
MinGW Debug: ctest 3/3; test_origin 403 passed
MSVC Debug:  test_origin 403 passed
```

## 后续

- [ ] 按 P0 顺序修复；
- [ ] 为 `BlockingQueue` / `CString` 补生命周期和溢出测试；
- [ ] 统一 C ABI 异常边界；
- [ ] 补充 `common` / `container` 的失败注入测试。

## 后续（2026-11-25）

修复提交 `d075c87` 已关闭本报告新增的全部 P0，以及 CString 和队列相关的连带项：

- COMMON-001/002/003：随 `working_env` 整体删除；
- CONTAINER-001/009：`BlockingQueue` 改用 `std::optional` 槽位，不再要求 `T` 默认构造；
- CONTAINER-002/007：`dequeue` 改阻塞锁，出队后通知 `_pushCv`；
- CONTAINER-003/004/010：随 `CString` 删除；
- CONTAINER-005：删除不存在的 `pop_front` 和重复出队；
- CONTAINER-008：`clear` 后 `notify_all`；
- LOGGING-003：删除 `root_logger_raw()`。

验证：MSVC x64 Debug 与 MinGW Debug 的 ctest 均 3/3 通过（`test_Container` 27、`test_Utils` 143、`test_Logging` 226）；`test_origin` 仍未注册。逐项状态与 `fixed_in` / `verification` 见 `docs/audit/findings.json`。

## 后续（2026-11-25，P1 / C ABI 批次）

提交 `b5e02e0` 关闭 CROSS-001 及其具体条目：

- COMMON-010：`origin_debug_logger_c` 整体 try/catch，`format == nullptr` 回退到 `fputs`；
- CROSS-001：`common` / `logging` 的 `extern "C"` 入口统一异常保护，指针回 `nullptr`、`bool` 回 `false`、`LogLevelC` 回 `OFF`；
- CONTAINER-006：`at/operator[]` 改为持锁返回 `T` 拷贝；
- UTILS-004/005：`get_entry_type` 改用 `symlink_status(path, ec)`，FIFO/SOCKET 分开映射；
- UTILS-006：`copy_file`/`rename_file` 保留目标路径探测错误；
- UTILS-007：路径辅助函数与 `join_paths` 包 try/catch，值返回失败为空串/false；
- UTILS-008：`ErrorCode` 声明统一补 `[[nodiscard]]`，修测试忽略返回值；
- UTILS-009：新增 `ErrorCode get_now_date_time(DateTimeSt&)`；
- LOGGING-013：`SinkImplBase::log/flush` 补 `catch (...)`。

验证：MSVC x64 Debug 与 MinGW Debug ctest 均 3/3（`test_Container` 28、`test_Utils` 145、`test_Logging` 231）；新增 symlink/FIFO/socket、非 `std::exception`、队列快照、`get_now_date_time` 测试。逐项 `fixed_in` / `verification` 见 `docs/audit/findings.json`。

## 后续（2026-11-25，P2 批次）

本批次关闭以下 P2；提交 `5806cf7` 后 fixed_in 指向该提交：

- UTILS-010/011：`FileWriter::write/write_line/flush` 返回 `ErrorCode`；构造函数不再抛异常，`to_absolute_path` 失败通过 `open()` 返回。
- UTILS-014：`write_text_file` 写失败统一使用 `generic_category()` + errno，并检查 `close()`。
- UTILS-016：`file_exists`/`dir_exists` 文档说明权限错误也返回 false。
- LOGGING-008：旋转后 `reopen` 返回值被检查，失败上报并抛 `runtime_error`。
- LOGGING-009：新增 `ErrorCode get_default_log_file(..., std::string&)`，默认路径失败保留具体错误。
- LOGGING-010：`SinkImplBase` 删除无调用的 `set_param_string`，`_paramStr` 构造后只读。
- LOGGING-014：`BasicFileSinkImpl` 用 `FileWriter` 的 ErrorCode 结果，保留解析/打开的具体错误。
- CONTAINER-011：补非平凡类型生命周期、`clear` 唤醒、容量边界测试。
- CROSS-002：容器生命周期/边界与 C API 异常测试已覆盖审计建议的注入点。
- LOGGING-012：标 `partial`，已有 sink 异常、非法路径、async 池销毁降级；仍缺真实磁盘写满、权限拒绝、旋转失败注入。

验证：MSVC x64 Debug 与 MinGW Debug 全量 ctest 3/3；test_Container 30、test_Utils 146、test_Logging 231。
