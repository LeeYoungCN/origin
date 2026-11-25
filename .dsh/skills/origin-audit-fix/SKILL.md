---
name: origin-audit-fix
description: 修复 Origin 审计记录中的 open 或 partial 问题。读取 docs/audit 的已记录证据，先复现，再做最小修复和回归测试，最后更新 findings.json 与报告状态。适用于用户明确要求修复某个审计项；不要用它重新全量扫描。
---

# Origin 审计项修复

按 `docs/audit/findings.json` 的既有证据修复问题，不重复全量扫描，不把“改完看起来对”当成 `fixed`。

## 输入与边界

- 输入：审计 ID（如 `CONTAINER-003`），或“按优先级选下一个”。
- 默认候选：`open` / `partial`，优先级 `P0 > P1 > P2 > P3`。
- 不处理 `decision`、`wontfix`、`fixed`，除非用户明确要求。
- 一次只修一个 ID；避免把多个无关问题混进同一提交。
- 未经用户要求不提交、不 force push。

## 1. 读取证据

先读：

```text
AGENTS.md
docs/audit/README.md
docs/audit/findings.json
```

再读该 ID 对应的报告段落和源码。记录：

- `id`、`severity`、`status`、`module`；
- `files[].path` 和 `files[].lines`（行号可能已漂移，以源码为准）；
- `impact`、`recommendation`、已有 `verification`。

如果 ID 不存在，停止并报告；不要临时发明 ID。

## 2. 选择条目

用户给了 ID 就按 ID；否则按以下顺序选：

1. `P0` 且 `open`；
2. `P0` 且 `partial`；
3. `P1` 且 `open` / `partial`；
4. 更低优先级。

跳过 `decision` / `wontfix` / `fixed`，除非用户指定。

## 3. 先复现

在改代码前确认问题仍存在：

- 读调用方和错误路径，确认不是只在孤立函数里“看起来危险”；
- 能写测试就先写一个会失败的回归测试；
- 不能自动复现时，记录最小手工复现步骤或明确的静态证据；
- 如果证据与当前代码不符，先报告差异，不要盲改。

复现结果要记录：命令、输入、实际结果、期望结果。

## 4. 做最小修复

- 只改与 finding 直接相关的代码；
- 保持公共 API/ABI，除非 finding 明确要求变更；
- 遵守 `AGENTS.md` 的约定：
  - 可失败接口用 `ErrorCode`，错误文本保持纯函数；
  - C ABI 不抛异常、不暴露 STL；
  - 公共接口不泄漏 `std::filesystem::path`；
  - 库内不打业务日志；
  - RAII，异步回调明确生命周期。
- 补回归测试，优先放在对应组件的 `test_Container` / `test_Utils` / `test_Logging`；
- 涉及 I/O、日志、并发、生命周期时，补失败路径或并发用例。

## 5. 验证

先跑定向测试，再跑完整矩阵：

```bash
source ./script/load_msvc_env.sh
cmake --preset=msvc_x64_debug
cmake --build --preset=msvc_x64_debug_build --parallel
ctest --preset=msvc_x64_debug_ctest --output-on-failure

./script/cmake_tools.sh -s win_mingw_debug --configure --build --ctest
./script/check_layers.sh
./script/check_tidy.sh
```

记录实际命令和结果。只跑了一个平台时，不能写“MSVC/MinGW 均通过”。

如果修复涉及 Release/调试日志，再补对应配置的验证；需要更完整的构建流程时使用 `origin-build-verify`。

## 6. 更新审计状态

只改该条目的状态，不删除历史：

- 完全修复：`status: "fixed"`，写 `fixed_in` 和 `verification`；
- 只修一部分：`status: "partial"`，在 `note` 写清剩余缺口；
- 修不动或判断不是问题：保持 `open`，或按用户确认改 `decision` / `wontfix`，不要伪装成 `fixed`。

`fixed_in` 优先写提交 hash；尚未提交时写 `working-tree`，并在最终回复里明确说明。

报告只追加“后续”说明，不覆盖原始结论。模板和字段示例见 `references/audit-status.md`。

如果问题总数或状态统计变化，同步更新 `docs/audit/README.md` 的“当前记录”。

## 7. 提交（仅在用户要求时）

- 提交信息用中文，例如 `fix(container): 修复 CString 截断后长度越界`；
- 代码和审计文档可以同一提交，也可以分开；`fixed_in` 必须指向实际包含修复的提交；
- 提交前确认没有无关改动。

## 8. 输出

最终报告包含：

| 项目 | 内容 |
|---|---|
| Finding | ID、级别、模块 |
| 根因 | 具体机制，不写“可能有问题” |
| 改动 | 文件、关键行为 |
| 测试 | 新增/修改的用例 |
| 验证 | MSVC / MinGW 命令与结果 |
| 状态 | `fixed` / `partial` / 保持 `open` |
| 剩余风险 | 未覆盖的平台、ABI 或并发场景 |

## 禁止

- 不重新全量扫描已有审计范围；
- 不在没有测试或明确证据时标记 `fixed`；
- 不删除、重写历史 finding；
- 不顺手重构无关代码；
- 不把环境失败写成代码修复。
