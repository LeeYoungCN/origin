# 审计状态更新参考

## `findings.json` 条目

修复后只更新原条目，不新增重复条目。字段示例：

```json
{
  "id": "CONTAINER-003",
  "severity": "P0",
  "status": "fixed",
  "module": "container",
  "title": "CString 截断后长度超过容量",
  "files": [
    {
      "path": "src/libs/container/include/origin/container/cstring.hpp",
      "lines": [46, 53]
    }
  ],
  "impact": "format_to_n 截断后 m_length 可能大于容量，下一次 append 下溢并越界。",
  "recommendation": "按实际写入长度更新 m_length，并补截断用例。",
  "verification": "MSVC x64 Debug ctest 全通过；MinGW Debug ctest 全通过；新增超长格式化与再次 append 用例。",
  "fixed_in": "<commit>"
}
```

## 状态

| 状态 | 使用条件 |
|---|---|
| `fixed` | 已修复，且有可追溯的提交/工作区改动和实际验证 |
| `partial` | 只修了一部分，`note` 写清剩余缺口 |
| `open` | 仍未修复，或复现失败、证据不足 |
| `decision` | 设计取舍，不是 bug |
| `wontfix` | 明确不修，写原因 |

不要为了“看起来完成”把 `open` 改成 `fixed`。

## `fixed_in`

- 优先写短提交 hash，例如 `c37a364`；
- 尚未提交时写 `working-tree`，并在最终回复里明确说明未提交；
- 不要写不存在的 hash，也不要把审计文档提交的 hash 当成代码修复提交。

## `verification`

写实际执行过的命令和结果，不写“应该没问题”。示例：

```text
MSVC x64 Debug: cmake --build ...；ctest 3/3 通过；
MinGW Debug: cmake_tools.sh -s win_mingw_debug --ctest；3/3 通过；
新增 case：CString.FormatTruncateThenAppend。
```

如果只验证了一个平台，明确写“仅 MSVC”或“仅 MinGW”。

## `partial`

```json
{
  "status": "partial",
  "note": "已修复读取路径；写入路径仍可能在 close 后失败，待补测试。"
}
```

## 报告追加

在对应报告末尾追加一节，不覆盖原始结论：

```markdown
## 后续（YYYY-MM-DD）

- 修复 `CONTAINER-003`：`std::format_to_n` 截断后按实际写入长度更新 `m_length`。
- 新增回归测试：超长格式化后再次 append。
- 验证：MSVC x64 Debug / MinGW Debug ctest 全通过；`fixed_in`：`<commit>`。
```

## README 统计

如果状态数量变化，同步更新 `docs/audit/README.md` 的“当前记录”：

```text
基线：<commit>。当前共 N 项：X open、Y partial、Z fixed。
```

只有新增扫描或状态总数变化时才改统计；单个 `partial` 的内部缺口变化不需要改总数。
