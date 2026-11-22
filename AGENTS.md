# Origin 项目约定

## 审计与历史结论

修改 `src/libs/utils`、`src/libs/logging`，或处理错误码、日志、异常边界相关代码前，先读：

1. `docs/audit/README.md`
2. `docs/audit/findings.json`
3. 对应日期的审计报告

`findings.json` 是问题状态的机器可读来源；报告记录证据和建议。不要把聊天历史当作持久上下文。

- `open` / `partial` 的条目不要重复扫描，先基于已有证据继续；
- 修复后更新 `findings.json` 和报告中的状态，并补充验证命令；
- `fixed` 条目必须能追溯到提交或明确的工作区改动；
- 历史报告只追加“后续”说明，不覆盖原始结论。

## 构建与验证

- 默认 Debug 构建并跑全部测试；
- MSVC 与 MinGW 至少各验证一次；
- 涉及文件 I/O、日志、异步生命周期时，补失败路径或并发测试；
- 提交前确认工作区没有无关改动。

## 文档

新增审计记录使用 `docs/audit/template.md`，文件名 `YYYY-MM-DD-<scope>.md`。
