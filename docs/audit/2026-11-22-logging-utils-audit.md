# 2026-11-22 logging / utils 审计

## 元信息

- 仓库：`origin`
- 基线：`c37a364`（`fix(utils,logging): 修复 P0 路径、时间、写入和生命周期问题`）
- 工作区：代码干净；本报告随审计记录更新
- 范围：`src/libs/utils`、`src/libs/logging` 的生产代码和公共头文件
- 方法：静态阅读 + MinGW/MSVC 构建 + 全量测试 + 少量编译期/二进制抽查
- 限制：
  - 没有对每个 I/O 失败做故障注入；
  - 没有做 ThreadSanitizer/ASan 全量运行；
  - 行号基于上述基线，后续重构可能偏移。

## 结论摘要

| 级别 | 数量 | 说明 |
|---|---:|---|
| P0 | 6 | 5 fixed，1 partial（LOGGING-003 raw API 兼容保留） |
| P1 | 8 | 异常契约、错误映射、C ABI、可观测性 |
| P2 | 14 | API 一致性、细节正确性、测试缺口 |
| 决策 | 4 | 调试日志策略、日志分层、FileWriter 方向、FATAL 语义 |

状态：21 `open` / 2 `partial` / 5 `fixed`。

总体判断：

- `utils` 的路径越界、修改时间、FileWriter 写入失败已修复；仍有异常泄漏和错误映射问题；
- `logging` 的 async 池失效、shutdown 幂等、根 logger 公共路径已修复；raw API 仍保留风险；
- Release 现在保留 `ERR/FATAL`，sink 异常通过 `ORIGIN_DEBUG_ERR` 输出；不再有 fallback logger API。

## 设计决策

### D-001 调试日志策略

| 构建 | 编译期最低级别 | 运行时默认 |
|---|---|---|
| Debug / RelWithDebInfo | TRACE | INFO |
| Release 默认 | ERR/FATAL | ERR |
| Release + `ORIGIN_DEBUG_LOG_ALL_LEVELS=ON` | TRACE | ERR（仍由 API 调整） |
| 测试 | 不再自动强制 TRACE | 继承构建类型 |

- `ORIGIN_DEBUG_LOG_ALL_LEVELS` 是 CMake 选项，默认 `OFF`；
- 该选项只决定哪些级别被编译进来，运行时过滤仍由 `origin_set_debug_logger_level` 控制；
- 不再有 `ORIGIN_ENABLE_TEST` 的强制 TRACE 分支；
- 调试日志始终输出 `文件:行号[函数]`；
- FATAL 仅在 Debug / RelWithDebInfo / ALL_LEVELS 下 abort，Release 默认只记录；
- sink 异常使用 `ORIGIN_DEBUG_ERR` 输出；若运行时级别为 OFF，仍可能被抑制；
- 不新增 fallback logger API。

状态：**已实现（`cd77a75`）**。

### D-002 基础库错误与日志分层

- 基础库返回 `ErrorCode`，不决定业务日志；
- 日志在调用边界记录一次；
- 库内只保留 DEBUG/TRACE 诊断，Release 仅保留 ERR/FATAL；
- 预期失败（文件不存在等）不应在库内打 ERR。

状态：**方向已定，调用点未清理**。

### D-003 FileWriter 错误接口

`write`/`write_line`/`flush`/`close` 目前返回 `void`，错误靠 `get_last_error()` 事后读取。建议改为返回 `ErrorCode`；这是 ABI 破坏，需版本决策。

状态：**待决策**。

## P0：优先修复

### UTILS-001 `string_view.data()` 当 C 字符串

- 状态：`fixed`（`c37a364`）
- 位置：
  - `src/libs/utils/src/filesystem/filesystem_utils_file.cpp:129`
  - `:247`
  - `:271`
  - `:318`
- 证据：

  ```cpp
  std::ofstream file(path.data(), ...);   // path 是 std::string_view
  ```

- 影响：`string_view` 不保证 `data()[size()] == '\0'`。传入子串视图时可能读越界，属于未定义行为。
- 建议：改用 `std::string(path)` 或 `fs::path(path)`；同一函数内只转换一次。
- 验证：新增“子串视图作为路径”的测试；ASan 下跑文件读写。

### UTILS-002 `get_file_modify_time` 返回创建时间

- 状态：`fixed`（`c37a364`）
- 位置：`src/libs/utils/src/filesystem/filesystem_utils_file.cpp:391`
- 证据：

  ```cpp
  FILETIME ftCreate = fileAttr.ftCreationTime;   // 应为 ftLastWriteTime
  ```

- 影响：Windows 返回创建时间，POSIX 返回 `last_write_time`，跨平台语义不一致；依赖修改时间的逻辑会错。
- 建议：改 `ftLastWriteTime`；补“创建后修改文件，时间应变化”的测试。

### UTILS-003 `FileWriter` 写入/刷新失败被当成成功

- 状态：`fixed`（`c37a364`）
- 位置：`src/libs/utils/src/filesystem/internal/file_writer_impl.cpp:132-164`
- 证据：

  ```cpp
  _errcode = ERR_COMM_SUCCESS;
  _stream << str;          // 没有检查 failbit/badbit
  _currSize += str.length();
  ```

  `flush()`、`close()` 同样不检查结果。
- 影响：磁盘满、句柄失效、只读介质时仍返回成功；日志会静默丢失。
- 建议：写入后检查 `_stream` 状态；失败设置错误码并停止累加大小；`flush/close` 检查结果。
- 验证：自定义 `streambuf` 模拟写失败；断言返回错误码且 `get_last_error()` 不是 SUCCESS。

### LOGGING-001 异步 logger 在 TaskPool 销毁后空指针解引用

- 状态：`fixed`（`c37a364`）
- 位置：`src/libs/logging/src/loggers/internal/async_logger_impl.cpp:50`、`:55`
- 证据：

  ```cpp
  _taskPool.lock()->log(shared_from_this(), logMsg);
  ```

- 影响：构造时检查过 `expired()`，运行期池被重置后 `lock()` 返回 `nullptr`，直接崩溃。
- 建议：每次 `lock()` 后判空；为空时降级同步 `backend_log`，或丢弃并计数。
- 验证：创建 async logger 后销毁 TaskPool，再写日志；应不崩溃且不丢日志或明确计数。

### LOGGING-002 TaskPool 重复 shutdown 可能死锁

- 状态：`fixed`（`c37a364`）
- 位置：`src/libs/logging/src/internal/task_pool.cpp:40-44`（析构调用）、`:65-80`（public shutdown）
- 证据：

  ```cpp
  TaskPool::~TaskPool() { shutdown(); }
  void TaskPool::shutdown() {
      for (...) _buffer.enqueue_wait(LogTask(TaskType::SHUTDOWN));
  }
  ```

- 影响：用户先调用 `shutdown()`，析构再调用一次；工作线程已退出，第二次 `enqueue_wait` 可能永久阻塞。
- 建议：加 `std::atomic<bool> _stopped`，`shutdown()` 幂等；析构只走一次；`enqueue_wait` 加停止检查。
- 验证：连续调用 `shutdown()` 两次；析构不挂起。

### LOGGING-003 `root_logger_raw()` 返回裸指针后解锁

- 状态：`partial`（`c37a364` 已修内部公共路径；raw API 兼容保留）
- 位置：`src/libs/logging/src/internal/registry.cpp:43-51`
- 证据：

  ```cpp
  std::lock_guard lock(_loggerMapMtx);
  return _rootLogger.get();   // 锁已释放
  ```

- 影响：另一线程 `remove_all()`/`set_root_logger()` 重置 `_rootLogger` 后，调用方继续使用裸指针，可能 UAF。
- 建议：公共路径统一用 `root_logger()` 的 `shared_ptr`；`root_logger_raw()` 降为内部短临界区使用。
- 验证：并发 `set_root_logger/remove_all` 与日志写入压力测试。

## P1：错误契约和可观测性

| ID | 状态 | 位置 | 问题 | 建议 |
|---|---|---|---|---|
| UTILS-004 | open | `filesystem_utils.cpp:29` | `fs::exists(path)` 是抛异常重载；断链 symlink 被误判为不存在 | 先 `symlink_status(path, ec)`，按 `ec` 映射；不要用 `exists` 短路 |
| UTILS-005 | open | `filesystem_utils.cpp:39`、`:57` | 系统错误统一压成 `ERR_COMM_SYSTEM_ERROR`；`fifo` 映射成 `SOCKET`，`socket` 分支缺失 | 统一 `ConvertSysEcToErrorCode`；补 FIFO/SOCKET 映射 |
| UTILS-006 | open | `filesystem_utils_file.cpp:173`、`:207` | `copy_file`/`rename_file` 忽略目标路径探测错误，权限错误被压成 NOT_FILE | 保留 `typeCode` 并原样返回 |
| UTILS-007 | open | `filesystem_utils_path.cpp:62`、`:105-107`、`:126-165` | `join_paths`/`to_absolute_path` 的 path 构造在 try 外；`get_directory` 等值返回函数可能抛异常 | path 构造移入 try；值返回接口明确失败语义，或新增 ErrorCode 版本 |
| LOGGING-004 | open | `logging_c.cpp:48`、`:61`、`:85`、`:98`、`:169`、`:258`、`:263` | 非法 `LogLevelC` 经 `c_to_cpp_log_level` 抛异常，可能穿过 C ABI | C 入口先校验范围，非法值返回默认/void，不抛 |
| LOGGING-005 | partial | `sink_impl_base.cpp:20-60` | sink 异常通过 `ORIGIN_DEBUG_ERR` 输出，但运行时可被 OFF 抑制；`FileWriter` 写失败仍被吞 | 先修 UTILS-003，再让 sink 检查写入结果 |
| UTILS-008 | open | `filesystem.h`、`date_time.h`、`process.h`、`thread.h` | 所有 `ErrorCode` 返回都没有 `[[nodiscard]]`，调用方可静默忽略 | 统一加 `[[nodiscard]]`，修调用点 |
| UTILS-009 | open | `date_time_utils.cpp:137-142` | `get_now_date_time` 忽略 `timestamp_to_date_time` 失败，返回全 0 时间 | 增加 ErrorCode 版本或 `Result<DateTimeSt>` |

### LOGGING-005 补充

当前代码已把 sink 的 `log/flush` 异常改为 `ORIGIN_DEBUG_ERR`：

```cpp
ORIGIN_DEBUG_ERR(
    "Sink log failed. [Param]: \"{}\". [Exception]: \"{}\".", _paramStr, ex.what());
```

Release 保留 ERR，所以默认可见；但它仍受 `origin_set_debug_logger_level` 过滤，若运行时设为 OFF 会被抑制。并且**没有解决 `FileWriter` 写失败**：`_stream << str` 失败时 sink 仍认为成功。因此状态是 `partial`，依赖 UTILS-003。

## P2：一致性和细节

| ID | 状态 | 位置 | 问题 | 建议 |
|---|---|---|---|---|
| UTILS-010 | open | `file_writer.h:22-32` | `write`/`flush` 返回 void，错误靠 `get_last_error()` 隐藏状态 | 改为返回 ErrorCode，保留 deprecated `get_last_error()` |
| UTILS-011 | open | `file_writer_impl.cpp:28-37` | 构造函数抛 `invalid_argument`；`to_absolute_path` 返回值被忽略 | 统一错误契约；至少检查 `to_absolute_path` |
| UTILS-012 | open | `filesystem_utils_path.cpp:156-165` | `is_path_too_long` 按 UTF-8 字节数判断，Windows 按 UTF-16 单元数 | 按平台原生字符数判断 |
| UTILS-013 | open | `process_utils.cpp:43-70` | `GetModuleFileNameA` 丢 Unicode；固定 `MAX_PATH_STD` | 改 W API + UTF-8 转换；支持长路径 |
| UTILS-014 | open | `filesystem_utils_file.cpp:335` | 写失败用 `errno` + `system_category()`，与其它处 generic 不一致 | 统一 errno/generic，或读取 `GetLastError` |
| UTILS-015 | open | `filesystem_utils_file.cpp:374-384` | UTF-8 转 UTF-16 失败时 `wpath` 为空，最终报系统错误而非路径非法 | 转换失败返回 `ERR_UTILS_PATH_INVALID` |
| UTILS-016 | open | `filesystem_utils_file.cpp:107-112`、`directory.cpp:35-40` | `file_exists`/`dir_exists` 忽略错误，权限错误也返回 false | 文档写清；需要区分时用 `get_entry_type` |
| LOGGING-008 | open | `rotating_file_sink_impl_base.cpp:80-92` | 旋转后 `reopen` 返回码被忽略，失败后继续用坏 writer | 检查 `reopen` 返回码并报警 |
| LOGGING-009 | open | `logging/src/internal/common.cpp:18-24` | `get_proc_path()` 失败返回空串，默认日志路径可能变成 `logs/.log` | 用 ErrorCode 版本；失败时明确报错 |
| LOGGING-010 | open | `sink_impl_base.cpp:88-91` | `set_param_string` 无锁，`log()` 持锁读，存在数据竞争 | 加锁或改为初始化后不可变 |
| LOGGING-011 | open | 多处 | `ConvertSysEcToErrorCode` 已打一条，调用方 catch 又打一条，Debug 下重复 | 只在调用边界打一次，或降为 TRACE |
| LOGGING-012 | open | tests | 缺少写失败、权限拒绝、旋转失败、async 销毁等失败注入 | 增加故障注入和并发测试 |
| LOGGING-013 | open | `sink_impl_base.cpp:23-28` | 只捕获 `std::exception`，非标准异常会逃出 sink | 视需要加 `catch (...)`；至少不让日志崩溃进程 |
| LOGGING-014 | open | `basic_file_sink_impl.cpp:34-59` | `to_absolute_path` 值返回吞错；失败只抛 “file empty” | 用 ErrorCode 版本并保留具体错误 |

## 建议修复顺序

1. **P0 正确性**：UTILS-001、UTILS-002、UTILS-003、LOGGING-001、LOGGING-002、LOGGING-003；
2. **错误契约**：UTILS-004、UTILS-005、UTILS-006、UTILS-007、LOGGING-004；
3. **可观测性**：UTILS-003 完成后让 sink 检查写入结果，闭合 LOGGING-005；
4. **API 收敛**：UTILS-008、UTILS-010、UTILS-011；
5. **测试与文档**：失败注入、并发、Release/opt-in 验证。

## 验证记录

已执行：

```text
MinGW Debug:
  cmake --preset=win_mingw_debug
  cmake --build --preset=win_mingw_debug_build
  ctest --output-on-failure
  -> 3/3 passed
  test_origin -> 403 tests passed

MSVC Debug:
  cmake --preset=msvc_x64_debug
  cmake --build ... --target test_origin
  test_origin -> 403 tests passed

Release 编译期抽查:
  默认构建：ERR 字符串保留，TRACE 字符串不保留
  ORIGIN_DEBUG_LOG_ALL_LEVELS=ON：TRACE 编译进来
```

尚未执行：

- ASan/TSan 全量；
- 写失败/权限失败/旋转失败注入；
- async logger 销毁压力测试。

## 后续

- [ ] 按 P0 顺序修复并逐项更新 `findings.json`；
- [ ] 决定 FileWriter 是否在 2.0.0 内做破坏性签名调整；
- [ ] 补失败注入测试；
- [ ] 清理 utils 中“预期失败也打 ERR”的调用点。

## 后续（2026-11-25）

LOGGING-003 已由 `partial` 改为 `fixed`：删除 `root_logger_raw()` 公共 API 及测试引用，
公共路径只保留 `root_logger()` 的 `shared_ptr`。MSVC x64 Debug 与 MinGW Debug 的 ctest
均 3/3 通过；逐项记录见 `docs/audit/findings.json`。
