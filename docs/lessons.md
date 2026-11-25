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
