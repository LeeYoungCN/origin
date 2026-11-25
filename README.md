# Origin

轻量、跨平台的 C++20 基础库集合，包含日志、容器、文件系统、时间、线程等常用能力。
面向 Windows（MSVC / MinGW / clang-cl）、Linux 和 macOS，支持作为独立项目构建，
也可以被其他项目通过 `find_package` 或 `FetchContent` 集成。

当前版本：**2.0.0**（开发中，见 [CHANGELOG.md](CHANGELOG.md)）

## 特性

- **C++20**，模块划分清晰：`Common`（契约层）+ `Container` / `Utils`（行为层）+ `Logging`（组件层）
- **多工具链**：MSVC、clang-cl、MinGW、GCC、Clang 全部纳入 CI 矩阵
- **CMake Presets**：一条命令完成 configure / build / test / package
- **可安装、可打包**：标准 CMake 安装树 + CPack（ZIP / TGZ），下游可 `find_package`
- **可嵌入**：支持 `FetchContent` / `add_subdirectory`，不会污染父项目的全局设置
- **稳定 ABI 边界**：公共接口只暴露 `std::string` 等标准类型，不泄漏 `std::filesystem::path`
- **错误处理**：可失败接口返回 `ErrorCode` + 纯函数错误文本；数据返回接口另有同名值返回
  重载，不关心错误原因时可直接取结果
- **C ABI**：日志与版本信息提供 C 接口，便于 C# / Python 等绑定
- **跨语言封装**：发布包提供稳定的 C ABI，可被 C#（P/Invoke）、Python（ctypes/cffi）等语言封装调用；
  跨语言边界统一使用 UTF-8 字符串、错误码和显式内存所有权，不泄漏 C++ 类型或异常

## 组件

| 组件 | 目标 | 内容 |
|---|---|---|
| Common | `Origin::Common` | 版本信息、错误码体系、调试日志、平台/编译器宏、公共类型与常量、C ABI 版本 |
| Container | `Origin::Container` | 阻塞队列（`BlockingQueue` 本身不保证线程安全；`ConcurrentBlockingQueue` 提供并发封装） |
| Utils | `Origin::Utils` | 文件系统（`origin/filesystem`）、时间（`origin/date_time`）、字符串（`origin/strings`）、线程（`origin/thread`）、进程（`origin/process`） |
| Logging | `Origin::Logging` | 同步/异步 logger、pattern formatter、文件 sink（basic / daily / rotating）、C API |

每个组件都有对应的 C++ 命名空间和公共头文件目录，例如：

```cpp
#include <origin/common/version.h>        // origin::version()
#include <origin/logging/logging.hpp>     // origin::logging::get_logger()
#include <origin/filesystem/filesystem.h> // origin::filesystem::normalize_path()
```

## 环境要求

| 项 | 要求 |
|---|---|
| C++ 标准 | C++20（C 源文件使用 C23） |
| CMake | `CMakeLists.txt` 最低 3.14；`CMakePresets.json`（v9）需要 **3.30+**，CI 使用 4.x |
| 生成器 | Ninja（Windows/MSVC、clang-cl）、MinGW Makefiles、Unix Makefiles |
| 测试 | GoogleTest 1.18（优先 `find_package(GTest)`，找不到时自动 FetchContent） |

## 快速开始

### Windows（MSVC）

```powershell
# 1) 导入 MSVC 环境（点源，只影响当前会话）
Set-ExecutionPolicy -Scope Process Bypass   # 仅在系统禁止运行脚本时需要
. .\script\load_msvc_env.ps1

# 2) 配置、构建、测试
cmake --preset=msvc_x64_debug
cmake --build --preset=msvc_x64_debug_build
ctest --preset=msvc_x64_debug_ctest
```

### Linux / macOS

```bash
cmake --preset=linux_gnu_debug        # 或 linux_clang_debug / darwin_clang_debug
cmake --build --preset=linux_gnu_debug_build
ctest --preset=linux_gnu_debug_ctest
```

### 统一入口：`script/cmake_tools.sh`

Git Bash / MSYS2 / Linux 下可用一个脚本完成全部操作，并自动处理 MSVC 环境：

```bash
./script/cmake_tools.sh --preset=list                     # 列出当前平台可用 preset
./script/cmake_tools.sh -s msvc_x64_debug --configure     # 配置
./script/cmake_tools.sh -s msvc_x64_debug --build         # 构建
./script/cmake_tools.sh -s msvc_x64_debug --ctest         # 跑 ctest
./script/cmake_tools.sh -s msvc_x64_debug --gtest=Version.*  # 按 gtest 过滤用例
./script/cmake_tools.sh -s msvc_x64_debug --pkg           # CPack 打包
./script/cmake_tools.sh --help                            # 全部参数
```

## 可用的 Preset

| 平台 | configure presets |
|---|---|
| Windows (MSVC) | `msvc_x64_debug` / `msvc_x64_release`、`msvc_x86_debug` / `msvc_x86_release` |
| Windows (MinGW) | `win_mingw_debug` / `win_mingw_release` |
| Windows (clang-cl) | `win_clang_cl_debug` / `win_clang_cl_release` |
| Windows (clang + MSVC/MinGW target) | `win_clang_msvc_*`、`win_clang_mingw_*` |
| Linux | `linux_gnu_*`、`linux_clang_*` |
| macOS | `darwin_clang_*` |

每个 configure preset 都配套 `*_build`、`*_ctest`，打包则用 `*_pkg`：

```bash
cpack --preset=msvc_x64_debug_pkg
# → out/pkg/msvc_x64_debug/origin-<完整版本>-<preset>.zip
```

## 构建选项

| 选项 | 默认 | 说明 |
|---|---|---|
| `ORIGIN_BUILD_TESTS` | `OFF` | 构建测试目标（presets 里默认 `ON`；ctest 只跑组件进程，见下文） |
| `ORIGIN_ENABLE_ASAN` | `OFF` | 开启 AddressSanitizer |
| `ORIGIN_INSTALL` | `ON` | 生成 install 与 export 规则 |
| `ORIGIN_DEBUG_LOG_ALL_LEVELS` | `OFF` | 编译进全部 `ORIGIN_DEBUG_*` 级别；运行时过滤仍由 `origin_set_debug_logger_level` 控制 |

> 这些选项在所有模式下统一使用 `ORIGIN_` 前缀，嵌入父项目时不会和父项目同名选项冲突。
> 另注意：所有 `*_build` preset 的 target 包含 `all` 和 `install`，`cmake --build --preset=*_build` 会顺带安装。

## 测试

- 聚合进程 `test_origin`：包含 Utils / Logging / Container / Version 的全部用例，但**没有注册到 ctest**，需要直接运行构建产物；
- 组件进程：`test_Container`、`test_Utils`、`test_Logging`，这三个由 `ctest` 执行；
- `ctest --preset=<preset>_ctest` 会同时写出文本日志和 JUnit 报告：

```
out/Testing/<preset>_ctest/Test_ALL.log
out/Testing/<preset>_ctest/Test_ALL.xml
```

## 安装与部署

```bash
cmake --install out/build/<preset> --prefix <install-prefix>
```

安装树结构：

```
<prefix>/
├── bin/                        # DLL（Windows）与 PDB
├── include/origin/...          # 公共头文件
└── lib/
    ├── libOrigin<Module>.so/.dylib   # Linux/macOS 共享库；Debug 带 d 后缀
    ├── Origin<Module>.lib            # Windows 导入库（无静态库）
    └── cmake/Origin/                 # OriginConfig.cmake 与导出目标
```

Container 是 `INTERFACE` 目标（纯头文件），没有对应二进制。Debug 构建输出名带 `d`（如 `OriginUtilsd`），安装导出仍使用 `Origin::Utils` alias。

Windows 下 DLL 与可执行文件同在 `bin/`；Linux/macOS 需要可执行文件带 RPATH
（`$ORIGIN/../lib`），或把 `<prefix>/lib` 加入 `LD_LIBRARY_PATH`。

## 下游集成

**安装包方式（推荐正式项目）**：

```cmake
find_package(Origin 2.0.0 CONFIG REQUIRED)
target_link_libraries(app PRIVATE Origin::Logging)
```

**源码方式（FetchContent，适合联调）**：

```cmake
include(FetchContent)
FetchContent_Declare(origin
    GIT_REPOSITORY https://github.com/LeeYoungCN/origin.git
    GIT_TAG        v2.0.0
    GIT_SHALLOW    TRUE)
set(ORIGIN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(origin)

target_link_libraries(app PRIVATE Origin::Logging)
```

完整说明（版本锁定、部署、常见问题）见 [docs/integration.md](docs/integration.md)。

## 版本管理

- 版本号唯一来源：`CMakeLists.txt` 里的 `ORIGIN_VERSION_FILE`；
- 发版流程：改版本号 → 更新 `CHANGELOG.md` → 打 tag `v<版本号>` → CI 校验两者一致；
- 非 tag 构建自动追加开发后缀：`2.0.0-dev+g<sha>[-dirty]`；
- 安装包的版本兼容策略为 `SameMinorVersion`：只有 `major.minor` 相同才视为兼容，
  破坏性变更会放在新的 minor；
- 运行期可查询：

```cpp
origin::version();                 // 完整版本串
origin::build_info();              // version / commit / dirty
origin::headers_match_binary();    // 头文件与二进制是否一致
```

## 目录结构

```
.
├── cmake/                 # 构建模块（版本、工具链、打包、FetchContent、IDE 信息）
├── config/                # 配置模板
├── docs/                  # 文档
│   ├── guide/             # 组件使用文档
│   └── audit/             # 审计记录
├── script/                # 构建、发布、格式化脚本
├── src/
│   ├── libs/              # Common / Container / Utils / Logging
│   └── tests/             # 聚合测试入口
├── CMakeLists.txt
├── CMakePresets.json
└── CHANGELOG.md
```

## 文档

| 文档 | 内容 |
|---|---|
| [docs/README.md](docs/README.md) | 文档索引 |
| [docs/architecture.md](docs/architecture.md) | 模块、错误模型、日志生命周期与 C ABI 约定 |
| [docs/status.md](docs/status.md) | 当前版本、审计基线、open P0 与文档偏差 |
| [docs/testing.md](docs/testing.md) | 测试目标矩阵、ctest 现状与补测指引 |
| [docs/guide/logging.md](docs/guide/logging.md) | 日志组件使用说明 |
| [docs/guide/date_time.md](docs/guide/date_time.md) | 日期时间工具 |
| [docs/guide/filesystem.md](docs/guide/filesystem.md) | 文件系统工具 |
| [docs/guide/system_error_code.md](docs/guide/system_error_code.md) | 错误码体系与 `std::error_category` |
| [docs/guide/platform_macros.md](docs/guide/platform_macros.md) | 平台/编译器宏 |
| [docs/guide/windows_sandbox_label.md](docs/guide/windows_sandbox_label.md) | Windows 沙箱完整性标签问题 |
| [docs/integration.md](docs/integration.md) | 下游集成与部署 |
| [CHANGELOG.md](CHANGELOG.md) | 版本变更记录 |

## 开发约定

- 代码格式：`.clang-format`（`./script/format_all.sh` 批量格式化）；
- 静态检查：`.clang-tidy`，提交前跑 `./script/check_tidy.sh`（默认只查改动文件；
  `--all` 全量、`--fix` 自动修复、`--no-werror` 只报告）；
- 模块分层：`common <- container / utils <- logging`，低层模块不允许 include
  高层；`./script/check_layers.sh` 可单独检查（CI 也会跑）；
- 拼写检查：`.cspell.json`；
- 编辑器：`.clangd` 指向 `out/build/compile_commands.json`，该文件由
  `check_tidy.sh` / VS Code 从当前 preset 的构建目录自动复制刷新；
- CI：`.github/workflows/clang-tidy.yml` 在 Windows / Linux / macOS 三平台跑同一套
  检查（不设 required，不阻塞合并；Windows 需 LLVM 22.1.8）；
- 提交前建议至少跑一遍：

```bash
./script/check_tidy.sh
./script/cmake_tools.sh -s msvc_x64_debug --configure --build --ctest
```

## 许可证

仓库当前未包含 `LICENSE` 文件；对外分发前请先补充许可证声明。
