# 更新日志

本项目版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

版本号的唯一来源是 **`CMakeLists.txt` 里的 `ORIGIN_VERSION_FILE`**（解析逻辑在 `cmake/project_version.cmake`）：

- 发版 = 一个把 `ORIGIN_VERSION_FILE` 改成新版本号、同时在本文加一节的提交，然后打**同名** tag（`v1.2.3`）；
- CI 只做**校验**：tag 与 `ORIGIN_VERSION_FILE` 不一致时构建直接失败，不再往编译里注入版本号；
- 非发布构建（本地、分支）由 CMake 自动把 git 状态追加成 `1.2.3-dev+g<短 sha>`（工作区脏还会带 `-dirty`），
  所以本地编出来的库不会自称已发布版本；
- 需要临时覆盖版本时用 `-DORIGIN_VERSION=x.y.z`，只影响当前构建目录。

版本位与 ABI 的关系：

| 改动 | 版本位 | SOVERSION（major） |
|---|---|---|
| 仅新增接口 / 类成员 | minor | 不变，可直接替换库 |
| 修复 bug、改实现 | patch | 不变 |
| 删除接口、改签名、改虚函数布局 | major | 变化，依赖方需重编 |

## [1.0.1] - 未发布

### 变更

- 版本号来源改为树内的 `ORIGIN_VERSION_FILE`，解析逻辑抽到 `cmake/project_version.cmake`；
  CI 从"注入"改为"校验 tag 与之一致"；
  非发布构建自动追加 `-dev+g<sha>`，本地构建不再谎报为已发布版本。
- CI 修复 Windows 下 `-DORIGIN_VERSION=1.0.0` 被 pwsh 拆成两个参数导致 configure 失败；
  矩阵改为显式 include 列表，并新增 `win_clang_cl_debug` 构建。
- 版本信息的公开接口收敛到 Common 一套：删除 `utils` / `logging` / `container` 各自的
  `version.h` 与 `version()` / `build_time()` / `compiled_build_time()` 入口，删除
  `origin::build_time()`。
- 构建时间不再暴露：它取自 configure 阶段（增量重建后不会更新），且会让同一份源码
  产生不同二进制，妨碍可复现构建。
- C 接口的版本查询从 logging 挪到 Common：删除 `origin_logging_version()` / `origin_logging_build_time()`，
  改用 `origin/common/c/version_c.h` 的 `origin_c_abi_version()`（C ABI 版本）、`origin_version()`、
  `origin_version_commit()`（扁平 getter，供 C#/Python 等绑定使用）。
- `origin::logging::version()` 这类写法统一改为 `origin::version()`。
- 顶层接入 CPack：`CPACK_GENERATOR=ZIP;TGZ`，包名 `origin-<完整版本>-<preset>`，
  输出目录由 `CPACK_PACKAGE_DIRECTORY` 指定（默认 `out/pkg/<preset>`，`-B` 可覆盖）；
  cpack 按 install 规则自行装到 staging，不依赖 `out/install`。
- `cmake_tools.sh` 的 `--install` 语义由"安装 component"改为"指定安装前缀"
  （`--install=<prefix>`，默认 `out/install/<preset>`）；仅当目标目录是 Origin 安装树时才清空。
- `cmake_tools.sh` 日志统一为英文；Windows 平台识别兼容 MSYS2 / Cygwin（`MSYS_NT-*`）。

### 新增

- `cmake_tools.sh --pkg[=<out-dir>]`：调用 CPack 生成 `origin-<完整版本>-<preset>.zip`
  （Windows 上不依赖外部 `zip` 命令），默认输出到 `out/pkg/<preset>`，打包后清理 `_CPack_Packages`。
- `cmake_tools.sh --gtest_target=<name>`：只跑指定的测试进程（默认聚合进程 `test_origin`）；
  配合 `--gtest=<filter>` 过滤用例，`--gtest=list` 列出用例。
- Windows Debug 构建随安装树一起安装 PDB（`$<TARGET_PDB_FILE>`），可单步进入库代码。
- 构建树新增 `Origin::Common` / `Origin::Utils` / `Origin::Logging` 别名，与安装树写法一致。
- `cmake_configure.conf` 新增 `project_version_full`（完整版本，含 `-dev+g<sha>`）与
  `cpack_package_directory`；`common_func.sh` 新增 `create_dir`。
- Windows 下的 DLL 嵌入 `VERSIONINFO` 资源（`cmake/version.rc.in` + `cmake/version_resource.cmake`）：
  资源管理器与 `GetFileVersionInfo()` 能看到文件/产品版本，数值部分取 `1.0.1.0`，
  字符串部分带开发后缀（如 `1.0.1-dev+g3df8f84-dirty`）。
- `origin::build_info()`：结构化返回 `version` / `commit` / `dirty`，需要分字段上报时
  不必解析版本字符串；C 侧对应 `origin_version()` / `origin_version_commit()`。
- 版本头只保留语义化版本：`ORIGIN_VERSION_STRING` / `origin::compiled_version()` 形如 `1.0.1`；
  开发版后缀（`-dev+g<sha>[-dirty]`）移到生成头 `origin/build_info.h`，由 `origin::version()` 体现。
  这样每次提交与 reconfigure 都不再改动 `version.h`，避免全量重编。
- `origin::headers_match_binary()`：判断头文件版本与运行期库版本是否一致，用于发现"头/库混装"。
- 版本信息测试：锁定宏与 `Version` 字段一致、头/库一致、C ABI 版本与 getter 一致。
- `script/check_clang.sh`：按平台选择 clang debug preset，一条命令完成 configure / build / ctest。

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
