# Origin 状态快照

> 这是**人工维护**的快照，不是实时状态。事实来源优先级：
> [findings.json](audit/findings.json) > 审计报告 > 本文 > 其它文档。
> 行号会随提交漂移，修复前以符号和源码为准。

## 1. 当前快照

| 项 | 值 |
|---|---|
| 快照日期 | 2026-11-25 |
| 分支 / 基线 HEAD | `master` / `1fa6636`（`docs: 增加跨语言封装说明`） |
| 工作区 | 无代码改动；本次仅文档与 skill 更新 |
| 版本 | `2.0.0`，未发布；已有 tag `v1.0.0`、`v1.0.1` |
| 审计基线 | `c37a364`（2026-11-22 全量审计） |
| 许可证 | 仓库暂无 `LICENSE`，对外分发前必须补 |

审计计数、证据和验证方式以 [audit/README.md](audit/README.md) 与 [findings.json](audit/findings.json) 为准，本文不重复统计表。

## 2. 版本与发布状态

- 版本唯一来源：根 `CMakeLists.txt` 的 `ORIGIN_VERSION_FILE`（当前 `2.0.0`）。
- 当前 HEAD 不等于 `v2.0.0` tag，构建会得到 `2.0.0-dev+g<sha>[-dirty]`；完整串只进 `build_info.h`。
- 发布条件：改版本号 → 更新 `CHANGELOG.md` → 打 `v2.0.0` tag，且 CI 校验 tag 与版本一致。
- `ORIGIN_VERSION` 的 `-D` 覆盖会留在 CMake cache，换版本前用 `-UORIGIN_VERSION` 清除。

## 3. 构建与测试健康

本次文档整改没有重新跑构建/测试；不要把本文当成“当前构建通过”的证明。

- 常规验证矩阵：MSVC x64 Debug + MinGW Debug；平台相关改动再补 Linux/macOS。
- ctest 只注册 `test_Container`、`test_Utils`、`test_Logging`；聚合进程 `test_origin` 会构建但**不在 ctest 中**。
- `cmake --build --preset=*_build` 会连带 install；CI 的 `cmake-multi-cmake-presets.yml` 覆盖 6 个配置，`clang-tidy.yml` 不阻塞合并，`cmake-multi-platform.yml` 已过时。

测试目标、补测位置和 CI 详情见 [testing.md](testing.md)。

## 4. 审计状态

只列仍 open / partial 的 P0。触发条件和规避建议是本文的增量，完整证据见对应报告。

| ID | 状态 | 触发条件 | 规避 |
|---|---|---|---|
| COMMON-001 | open | Linux 下 `readlink` 失败 | 写 `path[-1]`，可能破坏栈；不要依赖异常环境下的 working env 输出 |
| CONTAINER-001 | open | `BlockingQueue` 元素类型非平凡 | 对已构造对象 placement new，UB；暂用平凡类型或外部加锁 |
| CONTAINER-002 | open | 多线程争用 `ConcurrentBlockingQueue::dequeue` | `try_lock` 失败后仍访问队列；高并发下避免依赖该路径 |
| CONTAINER-003 | open | `CString` 容量不足时走 `append(std::format...)` | `m_length` 越界后下溢；预留容量或改用 `append_va` 路径 |
| LOGGING-003 | partial | 多线程 `set_root_logger` / `remove_all` 与 `root_logger_raw()` 并发 | 裸指针 UAF；公共路径改用 `root_logger()` 的 `shared_ptr` |

P1 与完整清单见 `findings.json`，本文不再展开。

## 5. 文档偏差记录

已修正项见 git 历史，不再逐条维护。仍待处理（低优先级）：

- `script/deploy_origin.sh`、`script/check_clang.sh`、`script/refresh_def_str.sh` 没有文档入口。
- C API `origin_create_rotating_file_sink` 实际构造 `DailyFileSink` 的问题已在 guide 标注为已知缺陷，代码未修。
- `test_origin` 作为聚合入口仍无 `add_test`，是否注册需要单独决策（会影响 ctest 时长和失败定位）。

## 6. 维护方式

- 发版、审计基线更新、P0 修复后更新本文的“快照”和 P0 表；不要在此抄审计统计。
- 不要手工修改 `findings.json` 里的状态来“让文档一致”；先改代码和验证，再改状态。
- 修复 finding 时用 `id` + 符号定位，不要只按本文行号改。
