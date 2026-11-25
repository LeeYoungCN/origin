# Origin 状态快照

> 这是**人工维护**的快照，不是实时状态。事实来源优先级：
> [findings.json](audit/findings.json) > 审计报告 > 本文 > 其它文档。
> 行号会随提交漂移，修复前以符号和源码为准。

## 1. 当前快照

| 项 | 值 |
|---|---|
| 快照日期 | 2026-11-25 |
| 分支 / 基线 HEAD | `master` / `1fa6636`（`docs: 增加跨语言封装说明`） |
| 工作区 | **有代码改动，未提交**：删除 `working_env` / `CString` / `root_logger_raw`，修复并发队列 |
| 版本 | `2.0.0`，未发布；已有 tag `v1.0.0`、`v1.0.1` |
| 审计基线 | `c37a364`（2026-11-22 全量审计）；工作区 54 项 = 35 open / 1 partial / 18 fixed |
| 许可证 | 仓库暂无 `LICENSE`，对外分发前必须补 |

审计计数、证据和验证方式以 [audit/README.md](audit/README.md) 与 [findings.json](audit/findings.json) 为准，本文不重复统计表。

## 2. 版本与发布状态

- 版本唯一来源：根 `CMakeLists.txt` 的 `ORIGIN_VERSION_FILE`（当前 `2.0.0`）。
- 当前 HEAD 不等于 `v2.0.0` tag，构建会得到 `2.0.0-dev+g<sha>[-dirty]`；完整串只进 `build_info.h`。
- 发布条件：改版本号 → 更新 `CHANGELOG.md` → 打 `v2.0.0` tag，且 CI 校验 tag 与版本一致。
- `ORIGIN_VERSION` 的 `-D` 覆盖会留在 CMake cache，换版本前用 `-UORIGIN_VERSION` 清除。

## 3. 构建与测试健康

本工作区已跑过 MSVC x64 Debug 与 MinGW Debug 的构建与 ctest。

- 常规验证矩阵：MSVC x64 Debug + MinGW Debug；平台相关改动再补 Linux/macOS。
- 2026-11-25 工作区：MSVC x64 Debug 与 MinGW Debug 的 configure/build/install 均通过，`ctest` 均 3/3（`test_Container` 27、`test_Utils` 143、`test_Logging` 226）。
- ctest 只注册 `test_Container`、`test_Utils`、`test_Logging`；聚合进程 `test_origin` 会构建但**不在 ctest 中**。
- `cmake --build --preset=*_build` 会连带 install；CI 的 `cmake-multi-cmake-presets.yml` 覆盖 6 个配置，`clang-tidy.yml` 不阻塞合并，`cmake-multi-platform.yml` 已过时。

测试目标、补测位置和 CI 详情见 [testing.md](testing.md)。

## 4. 审计状态

当前没有仍 open / partial 的 P0。本次工作区关闭了 5 条 P0：

| ID | 原问题 | 处理 |
|---|---|---|
| COMMON-001 | Linux `readlink` 失败写 `path[-1]` | 随调试接口 `working_env` 整体删除 |
| CONTAINER-001 | `BlockingQueue` 预构造后 placement new | 改用 `optional` 槽位，按需构造/析构 |
| CONTAINER-002 | `dequeue` 不检查 `try_lock` | 改为阻塞锁 + 出队通知 |
| CONTAINER-003 | `CString` 格式化截断后长度越界 | 随 `CString` 整体删除 |
| LOGGING-003 | `root_logger_raw()` 裸指针 UAF | 删除该公共 API |

证据和逐项状态见 [findings.json](audit/findings.json) 与 [audit/README.md](audit/README.md)。P1 与剩余清单不再在本文展开。

## 5. 文档偏差记录

已修正项见 git 历史，不再逐条维护。仍待处理（低优先级）：

- `script/deploy_origin.sh`、`script/check_clang.sh`、`script/refresh_def_str.sh` 没有文档入口。
- C API `origin_create_rotating_file_sink` 实际构造 `DailyFileSink` 的问题已在 guide 标注为已知缺陷，代码未修。
- `test_origin` 作为聚合入口仍无 `add_test`，是否注册需要单独决策（会影响 ctest 时长和失败定位）。

## 6. 维护方式

- 发版、审计基线更新、P0 修复后更新本文的“快照”和 P0 表；不要在此抄审计统计。
- 不要手工修改 `findings.json` 里的状态来“让文档一致”；先改代码和验证，再改状态。
- 修复 finding 时用 `id` + 符号定位，不要只按本文行号改。
