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

## [2.0.0] - 未发布

### 变更

- 错误处理统一为长期方案：可失败接口返回 `ErrorCode`，错误文本用
  `get_utils_err_msg(code)` / `get_comm_err_msg(code)`；公共 thread-local API
  （`get_thread_last_err*`）及其内部实现已彻底移除，错误只随返回值传递。
- `date_time` 的可失败接口改为 `ErrorCode` + 出参：`local_date_time`、
  `utc_date_time`、`timestamp_to_date_time`、`date_time_to_timestamp`、
  `get_month_*` / `get_weekday_*`、`format_time_string`、`append_time_string`、
  `format_time_buffer`；兼容重载已删除。格式化接口保留两套用法：值返回版
  （直接返回 `std::string` / `size_t`，`format` 与 `timeZone` 有默认值，失败返回
  空串/0）与 ErrorCode 版（所有参数显式传入）。
- `filesystem` 的读/路径接口改为 `ErrorCode` + 出参：`read_text_file`、
  `get_file_size`、`get_entry_type`、`get_curr_working_dir`、`join_paths`、
  `normalize_path`、`to_absolute_path`；兼容重载已删除。
- `process` / `strings` / `thread` 的可失败接口改为 `ErrorCode` + 出参：
  `get_proc_*`、`va_list_to_string`、`set_curr_thread_name`、`get_curr_thread_name`。
- `date_time` / `filesystem` / `process` / `strings` / `thread` 的数据返回接口补充
  值返回便捷重载：直接返回结果，失败时返回空串 / `0` / `UNKNOWN`，不需要写
  `ErrorCode` + 出参；需要区分错误原因时仍用原接口。
- `logging` 内部继续使用 utils 的值返回便捷接口（`format_time_string`、`join_paths`、
  `to_absolute_path` 等），不引入 `ErrorCode` 出参；文件重试失败的日志只记录文件路径。
- **BREAKING**：`filesystem` 的写操作（`create_file` / `delete_file` / `copy_file` /
  `rename_file` / `write_text_file` / `create_dir` / `delete_dir`）返回值由 `bool`
  改为 `ErrorCode`（成功为 `ERR_COMM_SUCCESS`）。
- **BREAKING**：`origin/utils/{filesystem_utils,date_time_utils,thread_utils,process_utils,
  string_utils}.h` 迁移为 `origin/<domain>/<domain>.h`（如 `origin/filesystem/filesystem.h`），
  `file_writer.h` 移到 `origin/filesystem/file_writer.h`；旧路径直接删除，下游需更新 include。
  命名空间（`origin::filesystem` 等）保持不变。
- **BREAKING**：`OriginConfigVersion.cmake` 的兼容策略由 `SameMajorVersion` 改为
  `SameMinorVersion`，1.0 的下游不会误匹配到 1.1（1.1 起头文件路径不兼容）。
- **BREAKING**：`origin::string` 命名空间改为 `origin::strings`，头文件
  `origin/string/string.h` 改为 `origin/strings/strings.h`，源文件改为 `strings.cpp`
  （复数形式表示工具集合，与 abseil 的 `absl::strings` 一致）。
- 文档整理：`doc/` 下文件名统一为小写下划线（`date_time.md`、`filesystem.md`、
  `system_error_code.md`、`windows_sandbox_label.md`），并刷新过期的命名空间、
  头文件路径与示例。
- 构建选项统一为 `ORIGIN_BUILD_TESTS` / `ORIGIN_ENABLE_ASAN`（原 `BUILD_TESTS` /
  `ENABLE_ASAN` 已移除），presets 与 `cmake_tools.sh` 同步更新。
- 编译选项按语言/目标收敛：删除与默认值重复的 `/MDd`、`/MD`、`-O3`、`-g3 -O0` 追加；
  `/permissive-`、`/Zc:__cplusplus`、`/Zc:preprocessor` 仅对 C++ 生效并排除 clang-cl；
  摘掉 MSVC 默认 flags 里的 `/W3`，避免 D9025（overriding `/W3` with `/W4`）。
- 警告按编译器选择且只作用于 Origin 的库目标：MSVC 使用 `/W4`，
  clang / clang-cl / GCC 使用 `-Wall -Wextra -Wconversion -pedantic`。
- 删除自定义 `DEBUG_MODE` / `NDEBUG_MODE` 宏；`ENABLE_TEST` 改名为
  `ORIGIN_ENABLE_TEST`，并在 `debug_logger.h` 派生 `ORIGIN_DEBUG_LOG_ENABLED`
  统一控制调试日志开关（Release 默认关闭，测试构建保持开启）。
- ASan 下同时清理 C flags 里的 `/RTC`；去掉置空 `CMAKE_THIRD_PARTY_PATH`。
- Windows 下 MinGW 产物去掉 `lib` 前缀，DLL 命名与 MSVC 一致。
- `CMakePresets.json` 与 `cmake_tools.sh` 对齐：显式写出 `CMAKE_INSTALL_PREFIX` /
  `CPACK_PACKAGE_DIRECTORY` / `ORIGIN_ENABLE_ASAN` / `ENV_PARAM_FILE`；`msvc_x86_*` 取消 hidden
  并注明需要 32 位工具链环境；补全 `preset_array` 里的 `win_clang_mingw_*` / `win_clang_cl_*`。

### 修复

- 支持被 `FetchContent` / `add_subdirectory` 嵌入：路径改用 `PROJECT_SOURCE_DIR`，
  `CMAKE_INSTALL_PREFIX` / `CMAKE_THIRD_PARTY_PATH` / `CPACK_PACKAGE_DIRECTORY` / `include(CPack)`
  只在顶层生效，测试与 ASan 选项统一使用 `ORIGIN_` 前缀，
  不再覆盖父项目的全局设置。
- 修复安装出来的 `OriginConfig.cmake` 缺少 `find_dependency(Threads)`，
  下游 `find_package(Origin)` 时报 `Threads::Threads` 目标找不到。
- `/W4` 暴露的死代码：`GetCppStandard` / `GetCStandard` 改为 `#elif` 链并补全返回值；
  `logging.cpp` 删除 `throw` 之后不可达的 `return`（9 处）；
  `filesystem_utils_path.cpp` 删除命名空间自引用 `using`。
- `cmake_tools.sh --preset=list` 输出为空、`--clean` 不带 `-s` 时会删到 `out/build` 根目录。
- CI：Windows runner 的 clang-cl 任务显式安装 LLVM 22（VS 18 的 MSVC STL 14.52 要求
  Clang 22+，镜像自带的 LLVM 20 会触发 STL1000 编译失败）。

### 新增

- `doc/integration.md`：下游集成指南（`find_package` / `FetchContent` / `add_subdirectory` /
  vcpkg 的用法、版本锁定策略、常见问题）。

- `script/load_msvc_env.ps1`：用 `vswhere` 定位 Visual Studio，执行 `vcvarsall.bat`
  把 PATH / INCLUDE / LIB 导入当前 PowerShell 会话（支持 `-Architecture x86`），
  之后可直接 `cmake --preset=msvc_x64_debug`；bash 侧继续用 `load_msvc_env.sh`。
- 测试工程统一：新增 `origin_add_component_tests(<lib>)` CMake 函数，组件测试只需
  列出源文件；CTest 默认以 `--gtest_shuffle` 运行，尽早暴露测试间的顺序依赖。

### 升级指南（1.0.x → 2.0.0）

本版含源码级不兼容变更（1.1.0 未发布，直接从 1.0.x 升到 2.0.0），升级时按下面几项调整：

1. **头文件路径**
   - `origin/utils/{filesystem_utils,date_time_utils,thread_utils,process_utils,string_utils}.h`
     改为 `origin/<domain>/<domain>.h`（如 `origin/filesystem/filesystem.h`）；
   - `file_writer.h` 改为 `origin/filesystem/file_writer.h`。
2. **命名空间**：`origin::string` 改为 `origin::strings`（头文件 `origin/strings/strings.h`）。
3. **错误返回**：可失败接口统一返回 `ErrorCode`，需要产出值的接口用末尾出参：

   ```cpp
   // 1.0.x
   const std::string text = origin::filesystem::read_text_file(path);

   // 2.0.0：ErrorCode 版
   std::string content;
   if (origin::filesystem::read_text_file(path, content) != ERR_COMM_SUCCESS) { /* ... */ }

   // 2.0.0：值返回便捷版（失败返回空串，不需要错误详情时用）
   const std::string text2 = origin::filesystem::read_text_file(path);
   ```

4. **filesystem 写操作**：`create_file` / `delete_file` / `copy_file` / `rename_file` /
   `write_text_file` / `create_dir` / `delete_dir` 由 `bool` 改为 `ErrorCode`，
   `if (delete_file(p))` 改成 `if (delete_file(p) == ERR_COMM_SUCCESS)`。
5. **移除的接口**：公共 `get_thread_last_err*`（线程局部错误状态）已删除，错误码只随返回值传递。
6. **date_time 默认参数**：`ErrorCode` 版格式化接口没有默认参数，需显式传 `format` / `timeZone`；
   不想写这些参数时用值返回重载（保留 `format = DEFAULT_TIME_FMT`、`timeZone = LOCAL`）。
7. **版本匹配**：`OriginConfigVersion.cmake` 使用 `SameMinorVersion`，下游改为
   `find_package(Origin 2.0 CONFIG REQUIRED)`；SOVERSION 从 1 变为 2（Linux/macOS 为 `.so.2`）。

## [1.0.1] - 2026-11-14

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
- `CMakePresets.json` 新增 `packagePresets`：`cpack --preset msvc_x64_debug_pkg`
  （以及 `linux_gnu_*_pkg` / `darwin_clang_*_pkg`）可直接打包，免去手写 `--config` 路径。
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
