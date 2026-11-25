# 失败分类与日志

## 环境问题

典型输出：

```text
cl.exe: command not found
The C compiler identification is unknown
rc.exe not found
```

处理：

1. 确认在 Git Bash / MSYS2 中执行了 `source ./script/load_msvc_env.sh`，而不是直接运行脚本。
2. 确认 `command -v cl.exe`、`command -v rc.exe` 有输出。
3. PowerShell 使用 `. .\script\load_msvc_env.ps1`。
4. 仍失败时报告“环境未就绪”，不要修改源码或 CMake 来绕过。

## 生成器或构建目录不匹配

典型输出：

```text
generator : Ninja does not match the generator used previously
```

处理：

- 确认 preset 的生成器和已有构建目录一致。
- 不要用 `rm -rf` 直接删；先征得用户同意，或用 `git status` 确认没有未保存内容。
- 不要把不同 preset 的 `FETCHCONTENT_BASE_DIR` 混用；共享依赖时只共享 `SOURCE_DIR`。

## 配置失败

看 CMake 的第一段错误，而不是末尾的 `Configuring incomplete`。常见原因：

- 编译器路径错误；
- 工具链文件或 preset 变量拼写错误；
- 依赖目录已有 `CMakeCache.txt`，与当前 generator 不匹配；
- 磁盘空间或权限问题。

## 编译与链接失败

- 编译：记录首个 `error C...` / `fatal error`，包含文件、行号和完整诊断。
- 链接：记录 `LNK...` / `undefined reference` 的符号，以及产生该符号的目标。
- 不要只贴最后一行；也不要被后续级联错误带偏。

定向构建单个目标：

```bash
cmake --build --preset=msvc_x64_debug_build --target test_Utils
```

MinGW 可加：

```bash
./script/cmake_tools.sh -s win_mingw_debug --build=test_Utils
```

## 测试失败

先看失败测试名和断言，再用过滤器复现：

```bash
ctest --preset=msvc_x64_debug_ctest -R 'Utils' --output-on-failure
ctest --preset=msvc_x64_debug_ctest --rerun-failed --output-on-failure
```

日志和 JUnit：

```text
out/Testing/<preset>_ctest/Test_ALL.log
out/Testing/<preset>_ctest/Test_ALL.xml
```

gtest 单目标：

```bash
./script/cmake_tools.sh -s win_mingw_debug --gtest-target=test_Logging --gtest='FileWriter.*'
```

## 工具或沙箱问题

CMake 长时间无输出、构建进程被杀、或文件访问被拒绝时：

- 先判断是环境/权限问题，不要把它当作代码失败；
- 保留原始错误和复现命令；
- 需要更大权限时，明确说明需要什么权限以及原因，不要静默绕过。
