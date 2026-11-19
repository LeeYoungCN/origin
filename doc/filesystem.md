# 文件系统工具（`origin::filesystem`）

`std::filesystem` 的薄封装层：对外只暴露 `std::string`（UTF-8），可失败接口统一
返回 `ErrorCode`，不向调用方抛异常。

设计说明与完整约定见头文件 `origin/filesystem/filesystem.h` 顶部注释。

## 头文件与命名空间

| 内容 | 头文件 | 命名空间 |
|---|---|---|
| 类型（`FileInfo`、`EntryType` 等） | `origin/common/types/filesystem_types.h` | **全局命名空间** |
| 常量 | `origin/common/constants/filesystem_constants.h` | `origin::filesystem` |
| 文件/目录操作 | `origin/filesystem/filesystem.h` | `origin::filesystem` |
| 文件写入器 | `origin/filesystem/file_writer.h` | `origin::filesystem` |

## 类型（全局命名空间）

```cpp
enum class PathType { Nonexistent, File, Directory };

enum class EntryType {
    NONEXISTENT = 0, FILE, DIRECTORY, SYMBOL_LINK,
    CHARACTER_DEVICE, BLOCK_DEVICE, FIFO, SOCKET, UNKNOWN
};

enum class Permission { None = 0, Read = 1 << 0, Write = 1 << 1,
                        Execute = 1 << 2, ReadWrite = Read | Write,
                        All = Read | Write | Execute };

enum class FileAttribute { None = 0, Hidden = 1 << 0, Readonly = 1 << 1, System = 1 << 2 };

using FileSize = size_t;

struct FileInfo {
    std::string path;      // 完整路径
    FileSize size;         // 文件大小（字节）
    EntryType type;        // 条目类型
    TimestampMs modifyTime;// 最后修改时间戳（毫秒）
};
```

## 常量（`origin::filesystem`）

| 常量 | Windows | Unix |
|---|---|---|
| `MAX_PATH_STD` | 260 | 4096 |
| `MAX_PATH_LONG` | 32767 | 65536 |
| `MAX_FILENAME` | 256 | 255 |
| `PATH_SEP` | `"\\"` | `"/"` |
| `ALT_PATH_SEP` | `"/"` | `""` |

## 接口

### 路径处理（纯词法，不访问磁盘）

| 接口 | 说明 |
|---|---|
| `get_entry_type(path, out)` | 出参 `EntryType`；符号链接返回链接本身 |
| `get_entry_type_str(type)` | 枚举转字符串 |
| `get_curr_working_dir(out)` | 当前工作目录 |
| `join_paths(parts, out)` | 拼接路径；空列表返回 `ERR_UTILS_PATH_INVALID` |
| `normalize_path(path, out)` | `lexically_normal`：折叠 `.`、`..`、重复分隔符 |
| `to_absolute_path(relPath, baseDir, out)` | 拼到 `baseDir`（传空串表示 cwd）后转绝对路径 |
| `get_directory(path)` | 父目录；`a/b/` → `a` |
| `get_filename(path)` | 文件名；`a/b/` → 空串 |
| `get_filename_stem(path)` | 去掉最后一段扩展名；`a.tar.gz` → `a.tar` |
| `get_extension(path)` | 含点的扩展名；`.gitignore` → 空串 |
| `is_absolute_path(path)` | 是否绝对路径 |
| `is_path_too_long(path)` | 按 UTF-8 字节数预检 |

### 文件操作

| 接口 | 说明 |
|---|---|
| `file_exists(path)` | 存在且为普通文件 |
| `create_file(path)` | 创建空文件；已存在则截断为 0 字节 |
| `delete_file(path)` | 删除文件；不存在视为成功 |
| `copy_file(src, dest, overwrite = false)` | 复制文件 |
| `rename_file(src, dest, overwrite = false)` | 移动/重命名 |
| `read_text_file(path, out)` | 读文本到 `out` |
| `read_binary_file(path, out)` | 读二进制到 `out` |
| `write_text_file(path, content, overwrite = false)` | 写文本；`overwrite=false` 为追加 |
| `get_file_size(path, out)` | 出参为字节数 |
| `get_file_modify_time(path, out)` | 出参为最后修改时间戳（毫秒） |
| `get_file_info(path, out)` | 出参为 `FileInfo` |

### 目录操作

| 接口 | 说明 |
|---|---|
| `dir_exists(path)` | 存在且为目录 |
| `create_dir(path, recursive = true)` | 创建目录 |
| `delete_dir(path, recursive = true)` | 删除目录 |

### FileWriter

```cpp
class FileWriter {
public:
    explicit FileWriter(std::string_view file);

    ErrorCode open(bool overwrite = true);
    ErrorCode reopen(bool overwrite = true);
    void close();
    void write(std::string_view str);
    void write_line(std::string_view str);
    void flush();
    [[nodiscard]] ErrorCode get_last_error() const;
};
```

适合高频写入场景（内部缓冲，减少系统调用）。

### 值返回便捷重载（忽略错误码）

不需要区分错误原因时，可以直接取返回值，失败时返回默认值：

| 接口 | 失败时 |
|---|---|
| `EntryType get_entry_type(path)` | 不存在 `NONEXISTENT`；读取失败 `UNKNOWN` |
| `std::string get_curr_working_dir()` | 空串 |
| `std::string join_paths(parts)` | 空串（空列表也返回空串） |
| `std::string normalize_path(path)` | 空串 |
| `std::string to_absolute_path(relPath, baseDir = "")` | 空串 |
| `std::string read_text_file(path)` | 空串 |
| `std::vector<uint8_t> read_binary_file(path)` | 空容器 |
| `FileSize get_file_size(path)` | `0` |
| `TimestampMs get_file_modify_time(path)` | `0` |
| `FileInfo get_file_info(path)` | `path`/`type` 有效，`size`/`modifyTime` 为 `0` |

## 错误处理

- 可失败接口统一返回 `ErrorCode`：成功为 `ERR_COMM_SUCCESS`，失败为具体错误码；
- 需要产出值的接口（`read_text_file`、`get_file_size`、`join_paths` 等）用出参返回，
  失败时出参不被修改；同名的值返回重载会直接返回结果，失败时返回默认值（见上表）；
- 错误文本用纯函数获取，不再依赖 thread-local：

```cpp
#include <origin/filesystem/filesystem.h>
#include <origin/utils/utils_error_code.h>

const ErrorCode rc = origin::filesystem::create_dir("logs/app");
if (rc != ERR_COMM_SUCCESS) {
    std::printf("create_dir failed: %s\n", get_utils_err_msg(rc));
}
```

## 示例

```cpp
#include <cstdio>

#include <origin/filesystem/filesystem.h>
#include <origin/utils/utils_error_code.h>

int main()
{
    std::string dir;
    if (origin::filesystem::to_absolute_path("logs", "", dir) != ERR_COMM_SUCCESS) {
        return 1;
    }
    if (const ErrorCode rc = origin::filesystem::create_dir(dir, true); rc != ERR_COMM_SUCCESS) {
        std::printf("create_dir failed: %s\n", get_utils_err_msg(rc));
        return 1;
    }

    std::string file;
    (void)origin::filesystem::join_paths({dir, "app.log"}, file);
    if (const ErrorCode rc = origin::filesystem::create_file(file); rc != ERR_COMM_SUCCESS) {
        std::printf("create_file failed: %s\n", get_utils_err_msg(rc));
        return 1;
    }
    if (const ErrorCode rc = origin::filesystem::write_text_file(file, "hello\n", true);
        rc != ERR_COMM_SUCCESS) {
        std::printf("write failed: %s\n", get_utils_err_msg(rc));
        return 1;
    }

    FileInfo info{};
    if (origin::filesystem::get_file_info(file, info) == ERR_COMM_SUCCESS) {
        std::printf("%s: %zu bytes, type=%d\n",
                    info.path.c_str(),
                    info.size,
                    static_cast<int>(info.type));
    }
    return 0;
}
```

## 注意事项

- 所有路径参数/返回值都是 UTF-8；Windows 内部转 UTF-16；
- 输入 `\` 和 `/` 都接受，返回值使用平台原生分隔符；
- `normalize_path`、`to_absolute_path` 都是纯词法操作，**不检查路径是否存在**，
  也不解析符号链接；
- `create_dir` / `delete_dir` 的 `recursive` 默认为 `true`；
- Windows 长路径（>260）需要系统启用长路径支持，`is_path_too_long` 只做预检。
