# 下游项目集成指南

本文说明别的项目如何引用 Origin、如何锁定版本，以及各种方式的注意事项。

## 选择集成方式

| 方式 | 适用场景 | 版本表达 | 是否能看到/改 Origin 源码 |
|---|---|---|---|
| `find_package` | 正式项目、CI 发布 | `find_package(Origin 2.0.0 CONFIG REQUIRED)` | 否 |
| `FetchContent` | 内部联调、需要单步进库代码 | `GIT_TAG v2.0.0` | 是（构建时下载） |
| `add_subdirectory` | 直接 vendor 源码到仓库 | 由目录内容决定 | 是 |
| vcpkg / Conan / 制品库 | 统一依赖管理 | 各自 recipe / manifest | 否 |

同一个下游工程里**不要混用** `find_package` 和 `FetchContent`/`add_subdirectory`：两套都会定义 `Origin::Common` 等目标，第二次定义会直接报 `target already exists`。

---

## 方式一：find_package（推荐）

### 1. 准备 Origin 的安装树

任选其一：

```bash
# 从源码安装
cmake --preset=msvc_x64_debug
cmake --build --preset=msvc_x64_debug_build
cmake --install out/build/msvc_x64_debug --prefix <install-prefix>

# 或直接用 CPack 产物解压
cpack --preset=msvc_x64_debug_pkg
# 生成 out/pkg/msvc_x64_debug/origin-<version>-<preset>.zip
```

### 2. 在下游工程里查找

```cmake
find_package(Origin 2.0.0 CONFIG REQUIRED)

target_link_libraries(app PRIVATE Origin::Logging)
```

安装树里包含 `lib/cmake/Origin/OriginConfig.cmake` 和 `OriginConfigVersion.cmake`，所以：

| 写法 | 接受的版本 |
|---|---|
| `find_package(Origin 2.0.0 CONFIG REQUIRED)` | `>= 2.0.0` 且 `< 2.1`（同一 major.minor） |
| `find_package(Origin 2.0.0 CONFIG REQUIRED EXACT)` | **只接受 2.0.0** |
| `find_package(Origin 1.0 CONFIG REQUIRED)` | 不匹配 2.0（要求 `major.minor` 相同） |

安装树不在默认搜索路径时，用下面任意一种方式指定：

```bash
cmake -DCMAKE_PREFIX_PATH=<install-prefix> ...
# 或
cmake -DOrigin_DIR=<install-prefix>/lib/cmake/Origin ...
```

### 3. Windows 上拷贝 DLL

`find_package` 只负责链接，运行时 DLL 需要自己处理。CMake 3.21+：

```cmake
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:app> $<TARGET_FILE_DIR:app>
    COMMAND_EXPAND_LISTS)
```

---

## 方式二：FetchContent（推荐联调）

```cmake
cmake_minimum_required(VERSION 3.20)
project(app CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # Origin 头文件含中文注释，MSVC 需要 /utf-8
    add_compile_options(/utf-8)
endif()

include(FetchContent)
FetchContent_Declare(origin
    GIT_REPOSITORY https://github.com/LeeYoungCN/origin.git
    GIT_TAG        v2.0.0        # 永远钉 tag 或 commit，不要用 master
    GIT_SHALLOW    TRUE)

# 嵌入时 Origin 的测试默认关闭；如需打开用 ORIGIN_BUILD_TESTS
set(ORIGIN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(origin)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE Origin::Logging)
```

本地调试时把 `GIT_TAG` 换成源码目录：

```cmake
FetchContent_Declare(origin
    SOURCE_DIR "E:/code/origin")   # 或 ${CMAKE_CURRENT_SOURCE_DIR}/../origin
```

### FetchContent 下必须注意的点

| 事项 | 说明 |
|---|---|
| **C++20** | Origin 的公开头文件用到 `std::format` 等，下游必须 `CXX_STANDARD >= 20` |
| **`/utf-8`（MSVC）** | Origin 头文件含中文注释；不传 `/utf-8` 会报 `C4819`，甚至 `#if/#endif` 不匹配的 `C1070`。Origin 自身构建会加，但那是目录级选项，**不会传给父项目** |
| **测试开关** | 统一使用 `ORIGIN_BUILD_TESTS`（默认 `OFF`）。旧名 `BUILD_TESTS` 已移除（CMake 会提示该变量未被使用），避免和父项目同名选项互相干扰 |
| **ASan 开关** | 嵌入时使用 `ORIGIN_ENABLE_ASAN` |
| **不要混用** | 同一工程里既 `find_package` 又 `FetchContent` 会因 `Origin::*` 目标重复而失败 |
| **DLL 位置** | Origin 的 `CMAKE_RUNTIME_OUTPUT_DIRECTORY` 只作用于自己的目标，DLL 输出到 `<父项目 build>/bin`；父项目的 exe 默认在 build 根目录，运行前需要把 bin 加进 `PATH`，或用 `$<TARGET_RUNTIME_DLLS>` 拷贝 |
| **版本号** | 从源码嵌入时显示为 `2.0.0-dev+g<sha>`（开发版）；钉 tag 构建才是干净的 `2.0.0` |
| **升级方式** | 改 `GIT_TAG` 即可，不需要 clean（CMake 会重新拉取） |

### 嵌入时 Origin 对父项目的影响（已收敛）

Origin 通过 `ORIGIN_TOP_LEVEL`（`CMAKE_SOURCE_DIR == PROJECT_SOURCE_DIR`）区分顶层与嵌入：

| 行为 | 顶层构建 | 嵌入父项目 |
|---|---|---|
| `CMAKE_INSTALL_PREFIX` 默认值 | 设为 `out/install/<preset>` | **完全不动父项目** |
| `CMAKE_THIRD_PARTY_PATH`（third_lib 接入脚本的根目录）/ `CPACK_PACKAGE_DIRECTORY` | 设置默认值 | 不设置 |
| `include(CPack)` | 生成 CPack 配置 | 不生成 |
| 拉取 googletest | `ORIGIN_BUILD_TESTS=ON` 时 | 仅 `ORIGIN_BUILD_TESTS=ON` 时 |
| `CMAKE_RUNTIME_OUTPUT_DIRECTORY` | `out/build/<preset>/bin` | 父项目的 `bin`（目录级设置，仅影响 Origin 目标） |
| `install()` 规则 | 安装到 Origin 安装树 | **仍会注册到父项目**（父项目安装时一并安装 Origin） |

> `install()` 规则目前没有开关。如果下游不希望把 Origin 一起安装，可以只安装自己的 target：
> `cmake --install <build> --target <your-target>`，或在父项目里不执行 `install`。

---

## 方式三：add_subdirectory（vendor 源码）

```cmake
set(ORIGIN_BUILD_TESTS OFF)
add_subdirectory(third_party/origin)
target_link_libraries(app PRIVATE Origin::Logging)
```

注意事项与 FetchContent 相同。适合"不想让构建过程联网"的场景，代价是源码进入你的仓库、升级要手动同步。

---

## 方式四：vcpkg / Conan / 内部制品库

- **vcpkg overlay port**：`version` 写 `2.0.0`，`REF`/`SHA512` 指到 `v2.0.0`；
- **Conan**：recipe 里 `version = "2.0.0"`，`sources` 钉 tag/SHA；
- **内部制品库**：上传 `origin-<version>-<preset>.zip`，下游按版本下载解压后设置 `CMAKE_PREFIX_PATH`。

无论哪种，原则都是：**版本号 + 不可变引用（tag/SHA）**，不要依赖"环境里碰巧装的那个 Origin"。

---

## 版本策略

- Origin 使用语义化版本：`MAJOR.MINOR.PATCH`，tag 为 `v<版本号>`（如 `v2.0.0`）；
- `OriginConfigVersion.cmake` 采用 `SameMinorVersion`：只有 `major.minor` 相同的版本才视为兼容，
  破坏性变更放在新的 major（如 2.0 与 1.x 不兼容），同一 `major.minor` 的补丁号之间保持兼容；
- **正式项目建议 `EXACT` 锁定**，或在下游集中管理一个变量：

```cmake
set(ORIGIN_VERSION_REQUIRED "2.0.0" CACHE STRING "Origin version to use")
find_package(Origin ${ORIGIN_VERSION_REQUIRED} CONFIG REQUIRED)
```

- 从源码嵌入时若不允许开发版，运行期检查：

```cpp
#include <string_view>
#include <stdexcept>
#include <origin/common/version.h>

if (std::string_view(origin::version()).find("-dev") != std::string_view::npos) {
    throw std::runtime_error("origin dev build is not allowed in production");
}
```

### 升级流程

1. 阅读 `CHANGELOG.md` 对应版本段（变更 / 修复 / 新增）；
2. 修改下游的版本号或 `GIT_TAG`；
3. 重新 configure + 全量测试；
4. 若 `MAJOR` 变化，检查 `SOVERSION`、DLL 名和链接项。

---

## 常见问题

| 现象 | 原因与解决 |
|---|---|
| `C4819` / `#if/#endif` 不匹配的 `C1070` | MSVC 缺 `/utf-8`，给下游 target 加 `add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)` |
| `find_package` 找不到 `OriginConfig.cmake` | 设置 `CMAKE_PREFIX_PATH` 或 `Origin_DIR` |
| `target already exists: Origin::Common` | 同时用了 `find_package` 和 `FetchContent`/`add_subdirectory`，只保留一种 |
| `LNK2038: RuntimeLibrary mismatch` | Debug / Release 库混链，或 CRT 设置不一致 |
| 运行时报找不到 `OriginLogging.dll` | DLL 不在 exe 目录或 PATH 中，用 `$<TARGET_RUNTIME_DLLS>` 拷贝 |
| 版本显示 `2.0.0-dev+g...` | 从源码嵌入且工作区有改动；发布构建请钉 tag 并用干净工作区 |
| 下游是 C++17 | Origin 头文件需要 C++20，无法降级使用 |
