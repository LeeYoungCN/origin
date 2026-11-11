# 更新日志

本项目版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

版本号的唯一来源是 **git tag**：打 `v1.2.3` 时 CI 通过 `-DORIGIN_VERSION=1.2.3` 注入构建，
产物包名、`origin::version()`、`ORIGIN_VERSION_STRING`、各组件 `version()` 都会同步；
本地不带参数构建时使用 `CMakeLists.txt` 里的默认值。

版本位与 ABI 的关系：

| 改动 | 版本位 | SOVERSION（major） |
|---|---|---|
| 仅新增接口 / 类成员 | minor | 不变，可直接替换库 |
| 修复 bug、改实现 | patch | 不变 |
| 删除接口、改签名、改虚函数布局 | major | 变化，依赖方需重编 |

## [1.0.0] - 2026-11-11

### 新增

- `apply_all(fun)`：遍历所有已注册 logger 的通用入口。基于快照实现，回调执行期间不持有注册表锁，
  可以在回调里安全调用 `get_logger` / `register_logger` / `remove_logger` 等接口。
- `log_all(source, level, ...)`：向所有已注册 logger 广播同一条日志，消息只格式化一次，
  返回通过各级别过滤的 logger 数量。提供「格式化字符串 / 字符串 / 字面量」三种重载。

### 行为变更

- `sink` 的默认级别由 `INFO` 改为 `TRACE`（与 spdlog 一致），即 sink 默认不再过滤，
  由 logger 这一层单独决定级别；依赖「sink 默认挡住低级别日志」的代码会看到更多输出。
- `set_level_all` / `flush_on_all` / `set_pattern_all` / `set_formatter_all` / `flush_all`
  统一为快照语义：遍历期间新注册的 logger 不会命中，被移除的 logger 在本次调用内仍然存活
  （此前为持锁遍历，会阻塞注册并存在回调死锁风险）。
- 移除 C 侧的 logger 枚举接口，C API 只保留具名操作；需要遍历请使用 C++ 的 `apply_all`。

### 修复

- 批量接口在持锁状态下调用 `apply_all`，因 `_loggerMapMtx` 为非递归锁而发生自我死锁。
- `apply_all` 的形参改为 `const std::function<...>&`，修复 clang 下 lambda 临时对象无法绑定非 const 引用的问题。
