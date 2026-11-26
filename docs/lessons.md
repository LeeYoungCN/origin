# 经验记录

记录非显而易见、容易复发的问题。审计问题的事实和状态仍以 `docs/audit/` 为准；这里记录原因、处理方式和下次如何避免。

> 记录前先判断层级：仅 Origin 特有的经验写到本文件；跨项目通用的原则写到全局 `$DSH_HOME/AGENTS.md`，不要混在一起。

## 记录格式

```markdown
### YYYY-MM-DD 标题

- 现象：
- 根因：
- 修复/规避：
- 验证：
- 适用范围：
- 关联：`docs/audit/...`、提交或 issue
```

## 条目

<!-- 新条目放在最上面；同类问题优先补充已有条目，不重复开新条目。 -->

### 2026-11-26 移除公共头的 windows.h 后，.cpp 不能单独包含 processthreadsapi.h

- 现象：Windows CI 的 Release 构建在 `src/libs/utils/src/thread_utils.cpp` 报 `winnt.h(169): error C1189: #error: "No Target Architecture"`；本机 Debug 增量构建未复现，说明依赖头文件的增量依赖可能漏掉这类问题。
- 根因：`thread_utils.cpp` 只包含 `<processthreadsapi.h>`，它依赖 `windows.h` 先定义目标架构宏（如 `_AMD64_`）。之前 `thread_types.h` 间接包含 `<windows.h>`，所以能编译；本次为了公共头去 Windows 宏污染删掉后，单独包含 SDK 子头就失去了架构定义。
- 修复/规避：在 Windows 分支包含 `<windows.h>`，由它引入 `GetCurrentThreadId` 及所需 SDK 子头；不要只包含 `processthreadsapi.h`/`fileapi.h` 这类依赖 `windows.h` 上下文的头。检查公共头删除 `windows.h` 时，所有原本靠它间接获得声明的 `.cpp`。
- 验证：本机 `msvc_x64_release` configure/build 通过，Release `ctest` 3/3（`test_Utils` 150、`test_Logging` 231，14.34s）；CI 同一 Release 构建由该提交修复。
- 适用范围：Windows MSVC（尤其完整 Release 构建）；MinGW 的 SDK 头实现不同，未复现，但同样建议包含 `<windows.h>`。Linux/macOS 不适用。
- 关联：2026-11-26 Windows CI 失败、`src/libs/utils/src/thread_utils.cpp`、`thread_types.h` 去 `<windows.h>` 的改动。

- 现象：Linux CI 的 `FilesystemErrorCodeApi.SymlinkAndSpecialFiles` 失败，`ASSERT_LT(socketPath.size(), sizeof(address.sun_path))` 报 `114 vs 108`。
- 根因：`sockaddr_un::sun_path` 只有约 108 字节（含结尾 NUL），而测试把 socket 建在构建目录下；CI 工作区路径较长，拼出的绝对路径超过上限。普通文件路径的 4096 字节上限在这里不适用。
- 修复/规避：测试改用 `std::filesystem::temp_directory_path()` 加 pid 生成短路径，并在 `memcpy` 前保留长度断言。另把 FIFO 断言改为“`mkfifo` 成功才校验”，因为 WSL 的 `/mnt` 挂载（drvfs）不支持 FIFO；CI 的 ext4 仍会执行该断言。
- 验证：WSL Linux `test_Utils` 331/331 通过；MSVC x64 Debug、MinGW Debug 的 `test_Utils` 均 146/146 通过。
- 适用范围：POSIX 下 `AF_UNIX` 测试或工具；Windows 无 `sun_path` 限制，不适用。FIFO 的 `/mnt` 限制仅针对 WSL drvfs，普通 Linux 文件系统可用。生产代码不应为规避该限制把业务 socket 放到临时目录，本条目只针对测试路径。
- 关联：2026-11-26 Linux CI 失败、`src/libs/utils/tests/src/test_error_code_api.cpp`。

### 2026-11-26 Windows 上 test_Logging 停在异步用例并以 0xCFFFFFFF/0xC0000409 退出，先排查二进制一致性

- 现象：ctest 报 `test_Logging` 失败，但 gtest 已跑完大部分用例，输出停在 `TestLoggingMPMC.Async_MPMC` 收尾，没有最终 `[ PASSED ]`/`[ FAILED ]` 汇总；退出码为 `0xCFFFFFFF`（Application Hang）或 `0xC0000409`，耗时明显长于正常值。
- 根因：未最终确认。两次都在整库重建、重新链接 `OriginLoggingd.dll` 后消失；更像测试可执行与共享库不同步，或异步任务池退出时序问题，而不是普通断言失败。
- 修复/规避：先全量构建目标而非只构建测试目标，确认测试二进制与 DLL 同批；再单独跑 `--gtest_filter=TestLoggingMPMC.*` 复现；最后才怀疑代码。临时 `fprintf` 曾定位到 `TaskPool::reset()`，但重新链接后不再复现，说明不能只凭一次挂起下结论。
- 验证：重建后 MSVC 与 MinGW 的 ctest 均 3/3，`test_Logging` 231 个用例通过；`TestLoggingMPMC.*` 连跑 5 次通过。
- 适用范围：Windows MSVC Debug 共享库 + ctest；`0xCFFFFFFF`/`0xC0000409` 只作为“异常退出”线索，不能直接等同于某段代码缺陷。Linux/MinGW 未复现，根因待确认。
- 关联：2026-11-25/26 P1/P2 批次验证。

### 2026-11-26 PowerShell 下 ctest 的 stderr 会触发 NativeCommandError

- 现象：`ctest --output-on-failure` 实际测试通过，PowerShell 仍抛 `NativeCommandError`，甚至让整条命令看起来失败；gtest/日志写到 stderr 时更明显。
- 根因：PowerShell 把原生进程的 stderr 当成错误记录；在管道/重定向组合里，`$LASTEXITCODE` 与错误流容易混在一起判断。
- 修复/规避：把 ctest 输出整体重定向到文件（`ctest ... *> out\ctest.log`），先看文件末尾和 `ctest` 的退出码；不要把 gtest 的 stderr 当成命令失败。bash 下无此问题。
- 验证：同一 ctest 命令在重定向后能正确读到 `100% tests passed` 与退出码。
- 适用范围：Windows PowerShell 5.1/7 调用会写 stderr 的原生程序；bash、cmd 不受影响。
- 关联：[testing.md](testing.md)、2026-11-25/26 构建验证。

### 2026-11-25 symlink_status 对“文件/子路径”返回 ENOTDIR，不能直接当目录类型错误

- 现象：Linux CI 上 `delete_dir("<可执行文件>/nonexistent")` 返回 `ERR_UTILS_NOT_DIRECTORY`，测试期望 `ERR_COMM_SUCCESS`；日志显示 `get_entry_type` 的 `symlink_status` 返回 errCode 20（ENOTDIR）。
- 根因：`get_entry_type` 从 `fs::exists` 改为 `fs::symlink_status(path, ec)` 后，只把 `no_such_file_or_directory` 映射为不存在。路径中间一段是普通文件时，POSIX 返回 ENOTDIR；`delete_dir` 忽略该错误，把 `UNKNOWN` 当成“不是目录”，破坏了“删除不存在的目录应成功”的幂等语义。`fs::exists` 对同一路径返回 false，所以旧实现没有暴露。
- 修复/规避：`get_entry_type` 同时把 `std::errc::not_a_directory` 归为 `NONEXISTENT` + `ERR_UTILS_NOT_FOUND`；`delete_dir` 因此回到成功路径。新增 `FilesystemErrorCodeApi.MissingPathUnderFileComponent` 回归测试，直接断言类型和删除结果。
- 验证：MSVC x64 Debug、MinGW Debug 的 `test_Utils` 均 146/146 通过；Linux CI 由同一回归测试覆盖。
- 适用范围：`std::filesystem::symlink_status` 替代 `exists` 做存在性判断的代码，POSIX 下尤其明显；Windows 上该路径通常报 `ERROR_PATH_NOT_FOUND`，但仍建议保留 `not_a_directory` 分支。不要把 ENOTDIR 一律映射为 `ERR_UTILS_NOT_DIRECTORY`，否则会破坏删除/创建等幂等操作。
- 关联：UTILS-004、提交 `b5e02e0`、2026-11-25 Linux CI 失败。

### 2026-11-25 DSH 沙箱下 CMake/Ninja 的 try_compile 挂起

- 现象：`cmake --preset=msvc_x64_debug` 停在 “Detecting C compiler ABI info” / `try_compile`，长时间不返回；`cmake`、`ninja` 进程存在但 CPU 接近 0，没有 `cl.exe` 子进程；删除 `.ninja_lock` 无效。
- 根因：DSH 的 workspace-write/只读沙箱限制子进程创建命名管道，Ninja 在 `try_compile` 中用 pipe 捕获编译器输出，子进程无法启动，父进程一直等待。不是编译慢，也不是 Ninja 锁残留。
- 修复/规避：构建和 ctest 从开始就使用完整权限（`sandbox_permissions: danger-full-access`）或放到普通终端/持久终端运行；不要反复删锁文件。若失败配置已把 `CMAKE_C_COMPILER` 写成相对 `cl.exe`，用完整 `cl.exe` 路径加 `--fresh` 重新 configure。
- 验证：同一 configure/build/install 命令在 full access 下通过；MSVC x64 Debug 与 MinGW Debug 的 ctest 均 3/3。
- 适用范围：Windows + DSH 受限沙箱 + CMake 的 Ninja 生成器；MinGW Makefiles 未复现，Linux/WSL 未复现。普通终端或无沙箱环境不需要此规避。
- 关联：2026-11-25 P0 修复；[testing.md](testing.md)、[status.md](status.md)。

### 2026-11-25 PowerShell 无法 dot-source load_msvc_env.ps1

- 现象：`. .\script\load_msvc_env.ps1` 被执行策略拒绝，当前会话仍未导入 MSVC 环境。
- 根因：PowerShell 默认 ExecutionPolicy（Restricted/AllSigned 等）禁止加载本地脚本；`load_msvc_env.ps1` 需要 dot-source 才能修改当前会话环境。
- 修复/规避：先执行 `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force`，再 dot-source；只影响当前进程，不修改机器或用户级策略。
- 验证：同一会话内 `cl.exe`、`cmake` 可用，configure/build 通过。
- 适用范围：Windows PowerShell 5.1 与 PowerShell 7，策略为 Restricted/AllSigned 时；bash 的 `source ./script/load_msvc_env.sh` 不受影响。
- 关联：`script/load_msvc_env.ps1`；构建说明见项目 `AGENTS.md`。

### 2026-11-25 Origin 构建/测试入口的非直觉点

- 现象：`cmake --build --preset=*_build` 会顺带 install；ctest 只出现 `test_Container`/`test_Utils`/`test_Logging`，看不到 `test_origin`；`check_tidy.sh` 会自己执行 configure，可能选中 Linux/WSL preset。
- 根因：所有 `*_build` 继承 `project_build_base`，targets 含 `install`；`src/tests/CMakeLists.txt` 只有 `add_executable`，没有 `add_test`；`check_tidy.sh` 在没有编译数据库时自动 configure。
- 修复/规避：改动后先跑组件 ctest，最后再单独运行 `test_origin` 聚合进程；构建/测试前先导入 MSVC 环境；跑 `check_tidy.sh` 前确认 preset 和编译数据库，避免误在 WSL 中配置。
- 验证：2026-11-25 MSVC 与 MinGW 的组件 ctest 均 3/3；`test_origin` 需手动运行，未注册进 ctest。
- 适用范围：Origin 仓库所有平台；具体命令和测试目标见 [testing.md](testing.md)。
- 关联：[testing.md](testing.md)、[status.md](status.md)、`CMakePresets.json`、`src/tests/CMakeLists.txt`。
