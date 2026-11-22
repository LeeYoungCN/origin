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

## 构建类型与调试日志

- CMake 只在当前配置定义 `ORIGIN_BUILD_DEBUG`、`ORIGIN_BUILD_RELEASE`、`ORIGIN_BUILD_RELWITHDEBINFO`、`ORIGIN_BUILD_MINSIZEREL` 之一。
- `ORIGIN_DEBUG_LOG_ALL_LEVELS` 只控制编译期可用级别；运行时过滤用 `origin_set_debug_logger_level`。
- Debug/RelWithDebInfo 默认运行时级别为 INFO；Release 默认 ERR。
- 调试日志格式始终包含 `文件:行号[函数]`，不要移除位置信息。
- FATAL 仅在 Debug/RelWithDebInfo/ALL_LEVELS 下 abort；Release 默认只记录。
- 判断 Origin 构建类型时不要依赖 `NDEBUG` 或 `_DEBUG`，使用上面的 `ORIGIN_BUILD_*` 宏。

## 构建与验证

- 默认 Debug 构建并跑全部测试；
- MSVC 与 MinGW 至少各验证一次；
- 涉及文件 I/O、日志、异步生命周期时，补失败路径或并发测试；
- 提交前确认工作区没有无关改动。

## 文档

新增审计记录使用 `docs/audit/template.md`，文件名 `YYYY-MM-DD-<scope>.md`。
