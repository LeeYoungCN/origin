# Origin 项目约定

Origin 是跨平台 C++20 基础库，组件包括 `Common`、`Container`、`Utils`、`Logging`。既支持独立构建，也被下游通过 `find_package` / `FetchContent` 集成，因此公共 API、ABI 和 CMake 行为都要保持稳定。发布包还会被 C#、Python 等语言通过 C ABI 封装调用，因此跨语言 ABI、字符串所有权和错误返回是硬约束。

## 项目速览

Origin 是 C++20 跨平台基础库，四个组件：

- `Common`（`SHARED`）：版本、错误码、调试日志、平台宏、C ABI 版本；
- `Container`（`INTERFACE`，纯头文件）：阻塞队列、C 字符串封装；
- `Utils`（`SHARED`）：文件系统、时间、字符串、线程、进程；
- `Logging`（`SHARED`）：logger、sink、formatter、异步线程池、C API。

依赖方向：`Common <- Container / Utils <- Logging`。Debug 产物带 `d` 后缀，没有静态库；所有 `*_build` preset 的 target 包含 `install`。

### 非平凡改动先读

1. [`docs/architecture.md`](docs/architecture.md)：模块、错误模型、日志对象图、所有权、C ABI；
2. [`docs/status.md`](docs/status.md)：当前 HEAD、审计基线、仍 open 的 P0；
3. [`docs/testing.md`](docs/testing.md)：ctest 实际跑什么、测试放哪里；
4. 任务相关 guide（`logging.md` / `filesystem.md` / `system_error_code.md` 等）。

### 五个常见陷阱

- `ctest` **不跑**聚合进程 `test_origin`，只跑 `test_Container` / `test_Utils` / `test_Logging`；
- `BlockingQueue` 本身不是线程安全的；并发场景用 `ConcurrentBlockingQueue`，并先看其 open 缺陷；
- `write_text_file` 要求目标文件已存在，不会创建文件或父目录；
- `create_logger` 不注册、不继承全局格式；root logger 在 Registry 构造时已注册，且可被 `remove_all` 删除；
- C ABI 的异常边界尚未完全统一（见 CROSS-001），C 侧不能假设失败一定返回错误码。

## 开始前先读

1. `README.md`：构建、preset、安装、集成和版本管理。
2. `docs/guide/` 下与改动相关的文档，例如 `logging.md`、`filesystem.md`、`system_error_code.md`；文档索引见 `docs/README.md`。
3. 修改 `src/libs/utils`、`src/libs/logging`，或涉及错误码、日志、异常边界、C ABI 时，先读：
   - `docs/audit/README.md`
   - `docs/audit/findings.json`
   - 对应的审计报告
4. 处理可能复发的问题前，先查 `docs/lessons.md`。

`findings.json` 是问题状态的机器可读来源；报告记录证据、建议和验证方式。不要把聊天历史当作持久上下文。

## 目录与分层

```text
src/libs/common       契约层：版本、错误码、平台宏、调试日志、C ABI
src/libs/container    行为层：阻塞队列、C 字符串封装
src/libs/utils        行为层：文件系统、时间、字符串、线程、进程
src/libs/logging      组件层：logger、formatter、文件 sink、C API
src/tests             聚合测试入口
```

依赖方向固定：

```text
common <- container
       <- utils
logging <- common / container / utils
```

- 低层模块禁止 include 高层模块的 `origin/<module>/...` 头文件。
- `src/tests` 是聚合入口，不参与分层限制。
- 公共头文件放在 `src/libs/<module>/include/origin/...`，实现放在 `src/libs/<module>/src`。
- 不确定时先跑 `./script/check_layers.sh`。

## 构建与测试

### Windows（MSVC）

在 Git Bash / MSYS2 中先导入 MSVC 环境，再执行 CMake：

```bash
source ./script/load_msvc_env.sh
cmake --preset=msvc_x64_debug
cmake --build --preset=msvc_x64_debug_build
ctest --preset=msvc_x64_debug_ctest
```

PowerShell 使用对应脚本：

```powershell
. .\script\load_msvc_env.ps1
cmake --preset=msvc_x64_debug
cmake --build --preset=msvc_x64_debug_build
ctest --preset=msvc_x64_debug_ctest
```

说明：

- 必须用 `source`（或 `.`）执行 `load_msvc_env.sh`；直接运行 `./script/load_msvc_env.sh` 只会修改子进程环境，当前 shell 仍然找不到 `cl.exe`。
- 手动运行 `cmake` / `cl.exe` / `ninja` 前必须先导入；否则可能找不到 `cl.exe`、`rc.exe` 或 Windows SDK。
- `script/cmake_tools.sh` 对 `msvc_*` preset 会自动 source 该脚本，手动调用 CMake 时仍需自己导入。
- 编译 x86 时先 `export ARCHITECTURE=x86`，再 source。

### Windows（MinGW / Git Bash / MSYS2）

```bash
./script/cmake_tools.sh -s win_mingw_debug --configure --build --ctest
```

### Linux / macOS

```bash
cmake --preset=linux_gnu_debug
cmake --build --preset=linux_gnu_debug_build
ctest --preset=linux_gnu_debug_ctest
```

也可用统一入口：

```bash
./script/cmake_tools.sh -s <preset> --configure --build --ctest
./script/cmake_tools.sh --preset=list
./script/cmake_tools.sh -s <preset> --gtest=Version.*
./script/cmake_tools.sh -s <preset> --gtest-target=test_Logging
```

约定：

- `CMakeLists.txt` 最低要求 CMake 3.14；`CMakePresets.json` v9 需要 CMake 3.30+。
- 主要选项：`ORIGIN_BUILD_TESTS`、`ORIGIN_ENABLE_ASAN`、`ORIGIN_INSTALL`、`ORIGIN_DEBUG_LOG_ALL_LEVELS`。
- 默认做 **Debug** 构建并跑全部测试。
- 改动至少验证 **MSVC 与 MinGW 各一次**；平台相关改动再补 Linux/macOS。
- 测试目标：聚合 `test_origin`，以及 `test_Container`、`test_Utils`、`test_Logging`；ctest 只注册后三个，`test_origin` 需直接运行构建产物。
- 涉及文件 I/O、日志、异步生命周期、并发时，补失败路径或并发测试。
- ctest 日志和 JUnit 报告在 `out/Testing/<preset>_ctest/`。
- 不提交 `out/`、`_deps/` 等构建产物。

## 代码约定

- 使用 C++20；C 源文件使用 C23。
- 不设置全局 `CMAKE_CXX_STANDARD`、全局编译选项或全局输出目录；通过 `target_compile_features` / `target_compile_options` 等 target 级属性表达，避免污染父项目。
- 公共接口不泄漏 `std::filesystem::path`；跨 DLL、跨语言边界使用 `std::string`（UTF-8）。
- 可失败接口返回 `ErrorCode` + 错误文本，并尽量提供值返回重载；错误文本应是纯函数，不依赖全局状态。
- 错误码接口加 `[[nodiscard]]`；调用方显式处理或显式丢弃。
- C ABI 入口不得让 C++ 异常穿出，必须捕获所有异常并返回错误码/空值；C ABI 不暴露 STL 类型。
- 面向 C#/Python 等宿主语言的导出接口使用 `extern "C"`，保持结构体布局、调用约定和句柄生命周期稳定；字符串统一 UTF-8，明确由哪一侧分配/释放。
- 库代码不打业务日志；日志组件自身也不应把业务语义写死。调试日志统一带 `文件:行号[函数]`。
- 优先使用 RAII；避免裸 `new/delete` 和长期持有裸指针；异步回调要明确生命周期归属。
- 风格以 `.clang-format` / `.clang-tidy` 为准；提交前跑：

```bash
./script/format_all.sh          # 需要时
./script/check_tidy.sh          # 默认检查改动文件
./script/check_layers.sh
```

## 构建类型与调试日志

CMake 只在当前配置定义以下宏之一：

- `ORIGIN_BUILD_DEBUG`
- `ORIGIN_BUILD_RELEASE`
- `ORIGIN_BUILD_RELWITHDEBINFO`
- `ORIGIN_BUILD_MINSIZEREL`

不要用 `NDEBUG` / `_DEBUG` 判断 Origin 构建类型。

- `ORIGIN_DEBUG_LOG_ALL_LEVELS` 只控制编译期可用级别，不是运行时全开。
- 运行时过滤使用 `origin_set_debug_logger_level`。
- Debug/RelWithDebInfo 默认运行时级别为 `INFO`；Release 默认 `ERR`。
- Release 默认编译 `ERR` / `FATAL`；`TRACE` / `DEBUG` / `INFO` 在 Release 下应编译为空。
- FATAL 仅在 Debug/RelWithDebInfo/ALL_LEVELS 下 abort；Release 默认只记录。
- 不要移除调试日志中的文件、行号、函数信息；缺失时输出 `<unknown>`。
- `assert.h` 使用 `!ORIGIN_BUILD_DEBUG` 关闭 Release 断言，不要依赖平台 `assert` 宏差异。

## 审计流程

修改 `utils` / `logging` 或处理错误码、日志、异常边界前：

1. 读 `docs/audit/README.md`、`docs/audit/findings.json` 和最新报告；
2. `open` / `partial` 条目先基于已有证据继续，不要重复全量扫描；
3. 修复后更新 `findings.json` 和报告状态；
4. `fixed` 必须能追溯到提交或明确的工作区改动，并写 `fixed_in` 与 `verification`；
5. 只修一部分标 `partial`，在 `note` 里写清剩余缺口；
6. 历史报告只追加“后续”说明，不覆盖原始结论；
7. 新增扫描复制 `docs/audit/template.md`，文件名 `YYYY-MM-DD-<scope>.md`，并同步 `findings.json` 和 README 表格。

## 项目 Skills

- `origin-build-verify`：按 MSVC/MinGW preset 构建并跑 ctest，分类失败；
- `origin-audit-fix`：按 `findings.json` 修复 open/partial 条目并更新审计状态。

## Git 网络操作

网络不稳定时，优先使用仓库脚本，不要反复手敲 `git push` / `git pull`：

```bash
./script/git_oper.sh pull
./script/git_oper.sh push
```

- `pull` 实际执行 `git pull --rebase`；执行前保持工作区干净，必要时先 commit 或 stash。
- `push` 实际执行 `git push -f`，失败后每 5 秒重试，直到成功或 `Ctrl+C`。
- 因为 push 是 force push，只能用于确认可以改写历史的个人分支；不要对共享 `main` / `master` 或他人分支使用。
- 需要非 force push、指定远端/分支或精细控制时，再用原生 `git` 命令。

## 经验固化

通用原则见 `$DSH_HOME/AGENTS.md`（本机 `C:\Users\a1175\.dsh\AGENTS.md`）。Origin 按以下方式落地：

- 先判断经验适用范围：
  - 仅 Origin 的模块、构建、平台、审计问题 → 写入 `docs/lessons.md`；
  - 跨项目通用原则 → 上提到全局 `AGENTS.md`，不要在项目里重复；
  - 影响公共 API/ABI、审计约定或构建规则 → 同时更新本项目 `AGENTS.md` 或 `docs/`。
- 每条至少包含：日期、现象、根因、修复/规避、验证方式、适用范围、关联。
- **适用范围**写清模块、平台、编译器/工具版本、触发条件，以及明确不适用的场景；无法确认标“待确认”。
- 先查 `docs/lessons.md` 是否已有同类记录；能补充就补充，不重复开条目。
- 审计问题的事实和状态仍以 `docs/audit/` 为准；`lessons` 只记录可复用经验，不替代 `findings.json`。
- 不记录密钥、token、隐私或客户数据。

## 版本、文档与提交

- 版本唯一来源：`CMakeLists.txt` 的 `ORIGIN_VERSION_FILE`。
- 发版：改版本号 → 更新 `CHANGELOG.md` → 打 tag `v<version>`；非 tag 构建自动追加开发后缀。
- 代码改动必须同步检查文档：公共 API/ABI、错误码、日志语义、CMake 选项、安装/集成方式、平台行为、测试命令有变化时，更新 `README.md`、`docs/` 和 `CHANGELOG.md`；不能只改代码。
- 仅内部重构且行为不变时，至少更新受影响注释；不要为了“以后可能有用”堆文档。
- 仓库当前没有 `LICENSE`；对外分发前先补许可证声明。
- 提交信息使用中文，沿用现有类型前缀，例如：
  - `fix(utils): ...`
  - `build(cmake): ...`
  - `docs(audit): ...`
  - `refactor(logging): ...`
  - `test(container): ...`
- 一个提交只做一类事情；提交前确认工作区没有无关改动。

## 完成标准

- 纯文档、Skill、注释等不涉及代码/CMake/测试的改动：不要求构建、ctest、`check_layers.sh` 或 `check_tidy.sh`；在结果中说明未跑的原因。
- 代码、CMake、测试或构建脚本改动：MSVC 与 MinGW 至少各构建、测试一次（平台不可用时说明原因）。
- 涉及库代码或公共头文件时，`check_layers.sh` 通过，改动文件跑过 `check_tidy.sh`。
- 新增/修改行为有对应测试，失败路径和并发场景有覆盖。
- 涉及审计范围时，`findings.json`、报告和验证命令已更新。
- 代码改动已同步更新受影响的 `README.md`、`docs/`、`CHANGELOG.md` 或注释；经验条目已写入正确层级（项目 `docs/lessons.md` 或全局 `AGENTS.md`），并写明适用范围。
- 不引入全局 CMake 副作用，不把构建产物或无关文件带进提交。
