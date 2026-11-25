# Origin 审计记录

本目录用于把一次性的代码扫描、问题结论和修复状态固化在仓库里，避免换会话、换工作区后丢失上下文。

## 给新会话/新工作区的读取顺序

1. 项目根目录的 `AGENTS.md`（会自动加载）指向本文件；
2. `docs/audit/README.md`（本文件）；
3. `docs/audit/findings.json`：机器可读的问题清单和状态；
4. 最新一份 `docs/audit/YYYY-MM-DD-*.md`：完整证据、建议和验证记录；
5. 需要时再回到源码核对行号。

只要工作目录指向本仓库，以上内容就随代码一起可用，不依赖聊天记录。

## 状态约定

| 状态 | 含义 |
|---|---|
| `open` | 已确认，未修复 |
| `partial` | 已修一部分，仍有明确缺口 |
| `fixed` | 已修复，必须记录提交或工作区改动 + 验证 |
| `decision` | 设计决策，不是待修 bug |
| `wontfix` | 明确不修，需写原因 |

`findings.json` 是状态主数据；Markdown 报告用于解释“为什么”和“怎么验证”。两者冲突时，以 `findings.json` 的 `updated_at` 较新者为准，并尽快同步。

## 当前记录

基线：`c37a364`。当前共 54 项：47 `open`、2 `partial`、5 `fixed`。

| 日期 | 范围 | 报告 | 条目与状态 |
|---|---|---|---|
| 2026-11-22 | 全项目：`common`/`container`/`logging`/`utils` | [2026-11-22-full-project-audit.md](2026-11-22-full-project-audit.md) | 新增 26 项，全部 open；另汇总既有 28 项 |
| 2026-11-22 | `utils` + `logging` 详细审计 | [2026-11-22-logging-utils-audit.md](2026-11-22-logging-utils-audit.md) | 21 open / 2 partial / 5 fixed |

## 54 项总览（2026-11-22 基线）

两份报告的关系：全项目报告只记录新增 26 项（Common 12、Container 11、跨模块 3），
[logging/utils 详细报告](2026-11-22-logging-utils-audit.md)记录既有 28 项（Utils 16、Logging 12）；
26 + 28 = 54，ID 不重复。下表是统一索引，证据和验证仍以两份报告为准。

`full` = [全项目审计](2026-11-22-full-project-audit.md)，`logging` = [logging/utils 审计](2026-11-22-logging-utils-audit.md)。

| ID | 状态 | 一句话 | 报告 |
|---|---|---|---|
| COMMON-001 | open | Linux `readlink` 失败后写 `path[-1]` | full |
| COMMON-002 | open | `origin_show_working_env` 的 C 入口可能抛异常 | full |
| COMMON-003 | open | `GetProcessPath` 固定 256 字节并忽略返回值 | full |
| COMMON-004 | open | `ERR_COMM_PARAM_OUT_OF_RANGE` 缺文案，`Unkown` 拼写错误 | full |
| COMMON-005 | open | `MAKE_ERROR` 不掩码，超宽值污染相邻字段 | full |
| COMMON-006 | open | `ATTR_ALIGNED` 在 MSVC 用 `__attribute__` | full |
| COMMON-007 | open | MinGW 把 `thread_local` 重定义为 `__thread` | full |
| COMMON-008 | open | warnings 的 push 没有对应 pop | full |
| COMMON-009 | open | 公共头包含 `<windows.h>`，ID 类型在全局命名空间 | full |
| COMMON-010 | open | `origin_debug_logger_c` 可能抛异常穿出 C ABI | full |
| COMMON-011 | open | `strings.cpp` 未判空，第二次 `vsnprintf` 返回值未检查 | full |
| COMMON-012 | open | `type_traits.h` 注释写 C++17，实际用 C++20 | full |
| CONTAINER-001 | open | `BlockingQueue` 在已构造对象上 placement new | full |
| CONTAINER-002 | open | `ConcurrentBlockingQueue::dequeue` 不检查 `owns_lock` | full |
| CONTAINER-003 | open | `CString::append(std::format...)` 截断后长度越界 | full |
| CONTAINER-004 | open | `vsnprintf` 负返回值被强转 `size_t` | full |
| CONTAINER-005 | open | `dequeue_try` 调用不存在的 `pop_front`，实例化即编译失败 | full |
| CONTAINER-006 | open | `operator[]`/`at` 解锁后返回元素引用 | full |
| CONTAINER-007 | open | 出队后不通知 `_pushCv`，生产者可能永久阻塞 | full |
| CONTAINER-008 | open | `clear()` 不唤醒等待线程，也不销毁元素 | full |
| CONTAINER-009 | open | 要求 `T` 默认构造，构造时预构造全部元素 | full |
| CONTAINER-010 | open | `CString` 在全局命名空间，const/转发有误 | full |
| CONTAINER-011 | open | 容器测试未覆盖非平凡 T、溢出和并发唤醒 | full |
| CROSS-001 | open | C ABI 异常策略不统一 | full |
| CROSS-002 | open | 测试缺少失败注入 | full |
| CROSS-003 | open | 构建宏与平台检测分散 | full |
| UTILS-001 | fixed | `string_view.data()` 当 C 字符串 | logging |
| UTILS-002 | fixed | `get_file_modify_time` 返回创建时间 | logging |
| UTILS-003 | fixed | `FileWriter` 写入/刷新失败被当成成功 | logging |
| UTILS-004 | open | `fs::exists` 抛异常，断链 symlink 误判 | logging |
| UTILS-005 | open | FIFO/SOCKET 映射错误，POSIX 错误被压成 `SYSTEM_ERROR` | logging |
| UTILS-006 | open | `copy_file`/`rename_file` 吞目标路径错误 | logging |
| UTILS-007 | open | 路径值返回接口可能抛异常 | logging |
| UTILS-008 | open | 所有 `ErrorCode` 接口缺 `[[nodiscard]]` | logging |
| UTILS-009 | open | `get_now_date_time` 吞错，返回全 0 时间 | logging |
| UTILS-010 | open | `FileWriter::write/flush` 返回 `void`，错误隐藏 | logging |
| UTILS-011 | open | `FileWriter` 构造抛异常，`to_absolute_path` 返回值被忽略 | logging |
| UTILS-012 | open | `is_path_too_long` 按 UTF-8 字节判断 | logging |
| UTILS-013 | open | `GetModuleFileNameA` 丢 Unicode，固定 `MAX_PATH` | logging |
| UTILS-014 | open | 写失败用 `errno` + `system_category`，与其它处不一致 | logging |
| UTILS-015 | open | UTF-8 转 UTF-16 失败时报系统错误而非路径非法 | logging |
| UTILS-016 | open | `file_exists`/`dir_exists` 忽略错误，权限错误也返回 false | logging |
| LOGGING-001 | fixed | 异步 logger 在 TaskPool 销毁后空指针解引用 | logging |
| LOGGING-002 | fixed | TaskPool 重复 `shutdown` 可能死锁 | logging |
| LOGGING-003 | partial | `root_logger_raw()` 返回裸指针后解锁 | logging |
| LOGGING-004 | open | C API 非法 `LogLevel` 抛异常，可能穿出 C 边界 | logging |
| LOGGING-005 | partial | sink 失败可观测性不足，`FileWriter` 写失败仍被吞 | logging |
| LOGGING-008 | open | 旋转后 `reopen` 返回码被忽略 | logging |
| LOGGING-009 | open | `get_proc_path` 失败导致默认路径变成 `logs/.log` | logging |
| LOGGING-010 | open | `set_param_string` 无锁，与 `log()` 存在数据竞争 | logging |
| LOGGING-011 | open | `ConvertSysEcToErrorCode` 重复打错误日志 | logging |
| LOGGING-012 | open | 测试缺少写失败、权限、旋转失败等注入 | logging |
| LOGGING-013 | open | sink 只捕获 `std::exception` | logging |
| LOGGING-014 | open | `basic_file_sink` 的 `to_absolute_path` 吞错 | logging |

### 跨模块条目的明细映射

`CROSS-*` 是归纳项，不是独立缺陷；闭合具体条目后再判断它是否关闭：

| 归纳项 | 依赖的具体条目 |
|---|---|
| CROSS-001 C ABI 异常策略 | COMMON-002、COMMON-010、LOGGING-004 |
| CROSS-002 失败注入 | CONTAINER-011、LOGGING-012、UTILS 中未覆盖的失败路径 |
| CROSS-003 构建/平台宏 | COMMON-006、COMMON-007 |

### 统一修复顺序

1. **P0 正确性**：COMMON-001、CONTAINER-001/002/003、LOGGING-003。
2. **错误契约**：UTILS-004/005/006/007、LOGGING-004、COMMON-002/010、CONTAINER-004~007。
3. **可观测性与 API 收敛**：UTILS-008/009、LOGGING-005、UTILS-010/011、LOGGING-008~010。
4. **测试与剩余 P2**：CONTAINER-011、LOGGING-012、CROSS-002，再按模块收口其余 P2。

### 验证来源

- [logging/utils 报告](2026-11-22-logging-utils-audit.md)实际执行了 MinGW Debug ctest 3/3、`test_origin` 403、MSVC Debug `test_origin` 403 和 Release 编译期抽查；
- [全项目报告](2026-11-22-full-project-audit.md)只做静态扫描，未重跑上述测试，其验证记录引用自前者。

## 如何新增一次扫描

1. 复制 `template.md` 为 `YYYY-MM-DD-<scope>.md`；
2. 在报告中写清基线（HEAD、工作区改动）、范围、方法、限制；
3. 每个问题分配稳定 ID，例如 `UTILS-001`、`LOGGING-003`；
4. 在 `findings.json` 的 `issues` 中追加条目，或在原条目上更新状态；
5. 更新本文件的“当前记录”表；
6. 提交时把报告、`findings.json`、README 一起提交。

## 修复后怎么更新

- 只改 `status`，不要删除历史问题；
- `fixed` 必须写：
  - `fixed_in`：提交 hash，或 `working-tree`；
  - `verification`：实际跑过的命令/测试结果；
- 如果只修了一部分，标 `partial`，并在 `note` 里写清剩余缺口；
- 设计决策放 `decisions`，不要伪装成 bug。

## 换工作区时

- 优先让新工作区直接指向本仓库（同一个 Git checkout 或 worktree），这样 `AGENTS.md` 和 `docs/audit/` 自动存在；
- 如果必须在另一个仓库工作，把本目录复制过去，并在那边的 `AGENTS.md` 增加指向；
- 不要把审计记录只放在会话缓存、桌面或临时目录；
- 提交到 Git 是最终固化手段，工作区文件只是草稿。

## 最小提示词

```text
先读 docs/audit/README.md 和 docs/audit/findings.json，
按 open/partial 条目继续，不要重复全量扫描。
```
