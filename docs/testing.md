# 测试与验证

本文说明 Origin 实际构建哪些测试目标、`ctest` 到底跑什么、以及新增用例该放哪里。
“文档说要测”不等于“CI 会跑”；以 CMake 注册结果为准。

## 1. 测试目标矩阵

| 目标 | 来源 | 是否注册到 ctest | 说明 |
|---|---|---|---|
| `test_origin` | `src/tests/` | **否** | 聚合入口：`main.cpp` + `test_version.cpp` + 所有组件的测试对象库 |
| `test_Container` | `src/libs/container/tests/` | 是 | 阻塞队列、并发队列、生命周期 |
| `test_Utils` | `src/libs/utils/tests/` | 是 | 时间、文件系统、FileWriter、线程、错误码接口 |
| `test_Logging` | `src/libs/logging/tests/` | 是 | logger、sink、formatter、C API、MPMC |
| `test_<lib>_obj` | `origin_add_component_tests` | 否 | 每个组件的 OBJECT 库，供聚合目标复用 |

`test_origin` 链接了全局属性 `ORIGIN_TEST_OBJ_LIST`，因此**包含全部组件用例**；但
`src/tests/CMakeLists.txt` 只 `add_executable`，没有 `add_test`。所以：

```bash
ctest --preset=msvc_x64_debug_ctest   # 只跑 3 个组件进程
```

不要根据 ctest 全绿判断“聚合进程也通过”。需要跑聚合进程时直接执行构建产物：

```bash
out/build/msvc_x64_debug/bin/test_origin.exe --gtest_shuffle
out/build/msvc_x64_debug/bin/test_origin.exe --gtest_filter='Version.*'
```

（Linux/macOS 去掉 `.exe`；路径以实际 preset 的构建目录为准。）

## 2. 常用命令

```bash
# 配置 + 构建 + ctest（组件进程）
cmake --preset=msvc_x64_debug
cmake --build --preset=msvc_x64_debug_build --parallel
ctest --preset=msvc_x64_debug_ctest --output-on-failure

# 只跑一个组件 / 一个用例
ctest --preset=msvc_x64_debug_ctest -R test_Utils --output-on-failure
ctest --preset=msvc_x64_debug_ctest --rerun-failed --output-on-failure

# 统一入口（会自动处理 MSVC 环境）
./script/cmake_tools.sh -s win_mingw_debug --configure --build --ctest
./script/cmake_tools.sh -s msvc_x64_debug --gtest-target=test_Utils --gtest='FileWriter.*'
```

日志和 JUnit 报告在 `out/Testing/<preset>_ctest/`：

```text
out/Testing/msvc_x64_debug_ctest/Test_ALL.log
out/Testing/msvc_x64_debug_ctest/Test_ALL.xml
```

注意：`*_build` preset 会顺带 install；如果只想编译不安装，用 `cmake --build out/build/<preset> --target <target>`。

## 3. 新增用例

### 3.1 放在哪个组件

| 改动 | 测试位置 |
|---|---|
| `src/libs/container` | `src/libs/container/tests/src/` |
| `src/libs/utils` | `src/libs/utils/tests/src/test_<area>/` |
| `src/libs/logging` | `src/libs/logging/tests/src/test_<area>/`，公共 mock 在 `src/detail/` |
| 版本/聚合入口 | `src/tests/test_version.cpp` 或 `src/tests/` |

Common 目前没有独立测试目标；涉及 Common 的用例通常放在 `test_Utils` / `test_Container` 的调用侧，或在 `src/tests/` 里补。

### 3.2 注册方式

1. 在对应组件 `tests/src/` 新建 `.cpp`，写 `TEST` / `TEST_F`；
2. 把文件加入该组件 `tests/CMakeLists.txt` 的 `SRC_LIST`（显式清单，不用 glob）；
3. `origin_add_component_tests` 会生成 `test_<lib>` 和 `test_<lib>_obj`，并 `add_test(... --gtest_shuffle)`；
4. 聚合目标会自动带上所有 `test_<lib>_obj`，不需要再改 `src/tests/CMakeLists.txt`。

测试文件命名沿用 `test_<主题>.cpp`；涉及平台差异时用仓库已有的 `COMPILER_*` / `OS_*` 宏，不要用 `#ifdef _MSVC`（该宏不存在，条件恒假）。

### 3.3 覆盖要求

按 `AGENTS.md` 的完成标准，以下改动必须补失败路径或并发用例：

- 文件 I/O：不存在、权限拒绝、目录当文件、长路径、非法 UTF-8；
- 日志：sink 打开/写入失败、异步队列满、`shutdown()` 后调用、生命周期；
- 并发：多生产者/多消费者、`clear` 唤醒、非平凡元素析构；
- 错误码：`ErrorCode` 版与值返回版语义一致，未命中文案回退；
- C ABI：非法枚举、空句柄、异常不穿出。

现有日志测试的 mock sink 在 `src/libs/logging/tests/src/detail/mock_sinks/`，可以直接复用。

### 3.4 全局状态与测试隔离

每个测试可执行文件的 `main()` 负责设定本进程的调试级别基线；当前各 `main()` 多为 `ORG_DBG_LVL_INFO`，但这不是强制值，可以按测试需要调整。用例可以临时修改级别（例如验证 `ORIGIN_DEBUG_ERR` 的上报路径），但必须在 `TearDown()` 或 RAII guard 中恢复到进入用例前的值，不能硬编码成另一个级别，也不能让状态泄漏到后续用例。

```cpp
OriginDbgLvl _level = origin_get_debug_logger_level();

void TearDown() override
{
    origin_set_debug_logger_level(_level);
}
```

同样的原则适用于其他进程级全局状态（默认 logger、注册表、环境变量）：谁修改，谁恢复。

## 4. 已知覆盖缺口

完整缺口以 [findings.json](audit/findings.json) 和审计报告为准，这里不维护第二份清单。与测试最相关的基线项是 CONTAINER-011、LOGGING-012、CROSS-002：
C 侧 rotating/daily/stderr 创建、失败注入和并发唤醒仍有缺口（`root_logger_raw` 已删除，对应风险随之消失）。
改到相关代码时先看对应 finding，不要因为“没有测试失败”就认为路径安全。

## 5. CI 现状

- `cmake-multi-cmake-presets.yml`：6 个构建配置（win_clang_cl_release/debug、win_mingw_release、linux_clang_release、linux_gnu_release、darwin_clang_release），阻塞合并；同时校验 tag 与版本一致。
- `clang-tidy.yml`：Windows/Linux/macOS 三平台静态检查，明确不设 required，**不阻塞合并**。
- `cmake-multi-platform.yml`：旧配置，选项名和测试流程已过时，不要作为依据。

CI 的构建配置不等于本地默认矩阵；本地改动至少验证 MSVC x64 Debug 与 MinGW Debug 各一次。

## 6. 验证记录模板

```text
基线：<commit>，工作区 <clean/dirty>
矩阵：msvc_x64_debug / win_mingw_debug
配置：通过/失败（首个错误）
构建：通过/失败（首个错误）
ctest：通过数/失败用例；是否跑了 test_origin（是/否）
定向用例：<命令与结果>
收尾：check_layers / check_tidy
未覆盖：<平台、并发、失败注入>
```

只有 ctest 实际跑过且通过，才能写“测试通过”；只跑了 `test_origin` 要注明“聚合进程通过，ctest 未覆盖”。
