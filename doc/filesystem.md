# 文件系统工具（`origin::filesystem`）

`std::filesystem` 的薄封装层：对外只暴露 `std::string`（UTF-8），错误通过返回值和
thread-local 错误码传递，不向调用方抛异常。

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
| `get_entry_type(path)` | `EntryType`；符号链接返回链接本身 |
| `get_entry_type_str(type)` | 枚举转字符串 |
| `get_curr_working_dir()` | 当前工作目录 |
| `join_paths(parts)` | 拼接路径；空列表返回空串并置错误码 |
| `normalize_path(path)` | `lexically_normal`：折叠 `.`、`..`、重复分隔符 |
| `to_absolute_path(relPath, baseDir = "")` | 拼到 `baseDir`（默认 cwd）后转绝对路径 |
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
| `delete_file(path)` | 删除文件 |
| `copy_file(src, dest, overwrite = false)` | 复制文件 |
| `rename_file(src, dest, overwrite = false)` | 移动/重命名 |
| `read_text_file(path)` | 读文本；失败返回空串 |
| `read_binary_file(path)` | 读二进制；失败返回空容器 |
| `write_text_file(path, content, overwrite = false)` | 写文本；`overwrite=false` 为追加 |
| `get_file_size(path)` | 字节数；失败返回 0 |
| `get_file_modify_time(path)` | 最后修改时间戳（毫秒） |
| `get_file_info(path)` | `FileInfo`；失败返回默认值 |

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

## 错误处理

- 修改类接口（`create_file` / `delete_file` / `copy_file` / `rename_file` /
  `write_text_file` / `create_dir` / `delete_dir`）返回 `ErrorCode`：成功为
  `ERR_COMM_SUCCESS`，失败为具体错误码；
- 查询类接口失败返回 `false` / 空字符串 / `0`；
- 错误码同时写入 thread-local 状态，可用 `origin/thread/thread.h` 里的
  `get_thread_last_err()` 和 `get_thread_last_err_msg()` 读取（诊断用途）。

```cpp
#include <origin/thread/thread.h>

if (!origin::filesystem::create_dir("logs/app")) {
    std::printf("create_dir failed: %s\n", get_thread_last_err_msg());
}
```

## 示例

```cpp
#include <cstdio>

#include <origin/filesystem/filesystem.h>
#include <origin/thread/thread.h>

int main()
{
    const std::string dir = origin::filesystem::to_absolute_path("logs");
    if (!origin::filesystem::create_dir(dir, true)) {
        std::printf("create_dir failed: %s\n", get_thread_last_err_msg());
        return 1;
    }

    const std::string file = origin::filesystem::join_paths({dir, "app.log"});
    if (!origin::filesystem::write_text_file(file, "hello\n", true)) {
        std::printf("write failed: %s\n", get_thread_last_err_msg());
        return 1;
    }

    const FileInfo info = origin::filesystem::get_file_info(file);
    std::printf("%s: %zu bytes, type=%d\n", info.path.c_str(), info.size,
                static_cast<int>(info.type));
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
