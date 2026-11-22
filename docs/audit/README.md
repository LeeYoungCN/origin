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

| 日期 | 范围 | 报告 | 未修复/部分修复 |
|---|---|---|---|
| 2026-11-22 | 全项目：`common`/`container`/`logging`/`utils` | [2026-11-22-full-project-audit.md](2026-11-22-full-project-audit.md) | 47 open / 2 partial |
| 2026-11-22 | `utils` + `logging` 详细审计 | [2026-11-22-logging-utils-audit.md](2026-11-22-logging-utils-audit.md) | 21 open / 2 partial / 5 fixed |

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
