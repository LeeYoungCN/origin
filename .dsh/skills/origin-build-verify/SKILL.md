---
name: origin-build-verify
description: 在 Origin 仓库发生代码、CMake 或测试改动后验证构建。默认跑 MSVC x64 Debug 与 MinGW Debug，分类环境、配置、编译、链接和测试失败，并给出可复现命令。只读问答或纯审计评审不要使用。
---

# Origin 构建验证

用仓库已有 preset 验证改动，默认验证 **MSVC x64 Debug** 和 **MinGW Debug**。不要自创 preset，不要为了“干净”删除 `out/` 或 build 目录。

## 输入与边界

- 可选参数：preset 名称、只跑某一个 preset、只跑某个测试。
- 默认矩阵：`msvc_x64_debug`、`win_mingw_debug`。
- 只报告和验证，不修改源码，不自动格式化，不提交；用户明确要求修复时才改代码。
- 不执行 `git clean`、`rm -rf out`、`--clean` 等破坏性操作；需要清理构建目录时先征得用户同意。

## 1. 记录基线

```bash
git rev-parse --show-toplevel
git rev-parse --short HEAD
git status --short
```

确认当前目录是仓库根目录。记录改动文件，后续只对相关目标做定向复跑。

如果 preset 不明确：

```bash
./script/cmake_tools.sh --preset=list
```

## 2. MSVC x64 Debug

在 Git Bash / MSYS2 中执行。必须先导入环境，`source` 不能省略：

```bash
source ./script/load_msvc_env.sh
command -v cl.exe
command -v rc.exe
cmake --preset=msvc_x64_debug
cmake --build --preset=msvc_x64_debug_build --parallel
ctest --preset=msvc_x64_debug_ctest --output-on-failure
```

PowerShell 对应命令：

```powershell
. .\script\load_msvc_env.ps1
cmake --preset=msvc_x64_debug
cmake --build --preset=msvc_x64_debug_build --parallel
ctest --preset=msvc_x64_debug_ctest --output-on-failure
```

如果 `cl.exe` 或 `rc.exe` 找不到，先按环境问题处理，不要改 CMake 或源码。

## 3. MinGW Debug

```bash
./script/cmake_tools.sh -s win_mingw_debug --configure --build --ctest
```

该脚本会输出 ctest 文本日志和 JUnit 报告，位置在：

```text
out/Testing/win_mingw_debug_ctest/Test_ALL.log
out/Testing/win_mingw_debug_ctest/Test_ALL.xml
```

## 4. 失败处理

按阶段分类，不要把环境失败误报成代码失败：

1. **配置失败**：编译器/生成器/SDK 找不到，或 CMake 自身报错。
2. **编译失败**：`error C...`、`fatal error`，记录首个真实错误的文件和行号。
3. **链接失败**：`LNK...`、`undefined reference`，记录缺失符号和目标库。
4. **测试失败**：记录失败测试名和断言，不要只看 ctest 汇总。
5. **工具/沙箱问题**：CMake 长时间无输出或进程被中断时，先检查环境和执行权限，不要先怀疑代码。

定向复跑：

```bash
ctest --preset=msvc_x64_debug_ctest -R '<test-regex>' --output-on-failure
ctest --preset=msvc_x64_debug_ctest --rerun-failed --output-on-failure
```

MinGW 可用：

```bash
./script/cmake_tools.sh -s win_mingw_debug --ctest='<test-regex>'
```

需要看具体 gtest 用例时：

```bash
./script/cmake_tools.sh -s win_mingw_debug --gtest-target=test_Utils --gtest='<case-filter>'
```

更细的分类和日志位置见 `references/failure-triage.md`。

## 5. 收尾检查

构建和测试通过后，对改动的 C++ 文件执行：

```bash
./script/check_layers.sh
./script/check_tidy.sh
```

- `check_tidy.sh` 默认检查改动文件；不要为了通过而自动 `--fix`，除非用户同意。
- 不要在验证过程中顺手改无关文件。
- 格式化只在用户要求时执行 `./script/format_all.sh`。

## 6. 输出

报告必须包含：

| 项目 | 内容 |
|---|---|
| 基线 | commit、工作区是否有改动 |
| 矩阵 | 实际运行的 preset |
| 结果 | 配置 / 构建 / 测试分别通过或失败 |
| 测试 | 通过数、失败测试名（如有） |
| 证据 | 首个错误、失败断言、日志路径 |
| 复现 | 可直接执行的失败命令 |
| 收尾 | `check_layers`、`check_tidy` 结果 |

只有在 ctest 实际跑过且通过时才能说“测试通过”。只跑了构建就明确写“仅构建通过，未跑测试”。
