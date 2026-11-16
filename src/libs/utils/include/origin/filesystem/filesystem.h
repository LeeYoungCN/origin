/**
 * @file filesystem_utils.h
 * @author LiYang
 * @brief 文件系统封装（std::filesystem 的薄封装层）
 * @version 0.2
 * @date 2025-07-22
 *
 * @copyright Copyright (c) 2025
 *
 * ## 为什么不用裸 std::filesystem
 *
 * 1. ABI 隔离：std::filesystem::path 在 MSVC 下是 std::wstring，在
 *    libstdc++ / libc++ / MinGW 下是 std::string，公共头暴露它会将 ABI
 *    绑死到具体标准库实现。本层对外只用 std::string（UTF-8）。
 * 2. 错误处理统一：不向调用方抛异常，失败返回 false / 空串 / 0，并把错误码
 *    写入 thread-local 状态（get_thread_last_err / get_thread_last_err_msg），
 *    与库内其它模块一致。
 * 3. 平台差异：Windows 长路径、UTF-8 ↔ UTF-16、分隔符归一化集中在这里处理。
 * 4. 可替换：将来更换实现（或去掉 std::filesystem）不影响调用方。
 *
 * ## 路径约定
 *
 * - 所有路径参数与返回值都是 UTF-8 字符串；Windows 内部再转 UTF-16 调用系统 API。
 * - 输入分隔符 `\` 和 `/` 都接受；返回值使用平台原生分隔符
 *   （Windows 为 `\`，POSIX 为 `/`）。
 * - 未特别说明时，相对路径相对于**当前工作目录**。
 * - 只有明确写了"要求存在"的接口才访问磁盘，其余是纯词法操作。
 * - 失败返回空字符串 / false / 0，错误码留在 thread-local 状态里。
 */
#ifndef ORIGIN_FILESYSTEM_FILESYSTEM_H
#define ORIGIN_FILESYSTEM_FILESYSTEM_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "origin/common/types/date_time_types.h"
#include "origin/common/types/filesystem_types.h"

namespace origin::filesystem {

/**
 * @brief 判断指定路径的文件系统条目类型
 *
 * 该函数使用C++17标准文件系统库，跨平台判断给定路径对应的
 * 文件系统条目类型（如文件、目录、符号链接等），或路径是否不存在。
 * 相比直接使用平台API，该实现具有良好的可移植性。
 *
 * @param path 待判断的路径字符串（支持相对路径和绝对路径）
 * @return EntryType 条目类型枚举值：
 *         - EntryType::Nonexistent：路径不存在
 *         - EntryType::File：普通文件
 *         - EntryType::Directory：目录
 *         - EntryType::SymbolicLink：符号链接（软链接）
 *         - EntryType::CharacterDevice：字符设备文件
 *         - EntryType::BlockDevice：块设备文件
 *         - EntryType::FIFO：FIFO管道文件
 *         - EntryType::Socket：套接字文件
 *         - EntryType::Unknown：无法识别的类型或获取信息失败（如权限不足）
 *
 * @note 函数内部处理了符号链接的特殊情况：返回链接本身的类型，
 *       而非链接指向的目标类型。如需获取目标类型，需先解析链接。
 */
EntryType get_entry_type(std::string_view path);

const char* get_entry_type_str(EntryType type);

inline const char* get_file_mode_str(bool overwrite)
{
    return overwrite ? "overwrite" : "append";
}

/**
 * @brief 获取当前工作路径。
 *
 * @return 工作路径
 */
std::string get_curr_working_dir();

/**
 * @brief 拼接多个路径片段为完整路径
 * @param parts 路径片段列表
 * @return 拼接后的完整路径字符串
 * @note 自动处理不同平台的路径分隔符
 * @note 空列表返回空串并设置 ERR_UTILS_PATH_INVALID；片段中若出现绝对路径，
 *       会替换掉前面已拼接的部分（std::filesystem::operator/= 语义）
 */
std::string join_paths(const std::vector<std::string>& parts);

/**
 * @brief 纯词法规范化路径（不访问磁盘）
 *
 * 等价于 std::filesystem::path(path).lexically_normal()：
 * - 折叠 `.` 段和多余的分隔符；
 * - 尽量折叠 `..`（在根目录或相对路径开头无法折叠时保留）；
 * - 不解析符号链接，不检查路径是否存在，不改变"相对/绝对"属性；
 * - 空串返回空串。
 *
 * @param path 原始路径（UTF-8）
 * @return 规范化后的路径；失败返回空串并设置错误码
 * @see to_absolute_path 需要绝对路径时使用
 */
std::string normalize_path(std::string_view path);

/**
 * @brief 将路径拼到基准目录后转为绝对路径
 *
 * 过程：base = baseDir（为空时取 get_curr_working_dir()）→ base / relPath
 * → std::filesystem::absolute → lexically_normal。
 *
 * - relPath 为空时直接返回空串（不修改错误码）；
 * - relPath 为绝对路径时忽略 base（operator/= 语义）；
 * - 不检查路径是否存在，不解析符号链接；
 * - baseDir 为相对路径时按当前工作目录解析；
 * - 结果中的 `.`、`..` 已按词法折叠。
 *
 * @param relPath 相对路径（也允许传绝对路径）
 * @param baseDir 基准目录（默认为当前工作目录）
 * @return 绝对路径；失败返回空串并设置错误码
 * @see normalize_path 只做词法规范化、不转绝对路径
 */
std::string to_absolute_path(std::string_view relPath, std::string_view baseDir = "");

/**
 * @brief 获取路径中的目录部分（父目录路径）
 * @param path 完整路径
 * @return 父目录路径
 * @note 与 std::filesystem::path::parent_path() 一致：`a/b/` 的父目录是 `a`，
 *       根目录的父目录为空串。
 */
std::string get_directory(std::string_view path);

/**
 * @brief 获取路径中的文件名（含扩展名）
 * @param path 完整路径
 * @return 带扩展名的文件名
 * @note 与 std::filesystem::path::filename() 一致：以分隔符结尾的路径
 *       （如 `a/b/`）返回空串。
 */
std::string get_filename(std::string_view path);

/**
 * @brief 获取路径中的文件名（不含扩展名）
 * @param path 完整路径
 * @return 不带扩展名的文件名
 * @note 只去掉最后一段扩展名：`archive.tar.gz` → `archive.tar`。
 */
std::string get_filename_stem(std::string_view path);

/**
 * @brief 获取文件的扩展名（含.）
 * @param path 完整路径或文件名
 * @return 文件扩展名（如".txt"），无扩展名则返回空字符串
 * @note 含前导点；以点开头的文件名（如 `.gitignore`）没有扩展名，返回空串。
 */
std::string get_extension(std::string_view path);

/**
 * @brief 判断路径是否为绝对路径
 * @param path 待判断的路径
 * @return 是绝对路径返回true，否则返回false
 */
bool is_absolute_path(std::string_view path);

/**
 * @brief 判断路径长度是否超过系统限制
 * @param path 待检查的路径
 * @return 超过限制返回true，否则返回false
 * @note 按 UTF-8 字符串字节数估算（Windows 用 MAX_PATH_LONG，其它平台用
 *       MAX_PATH_STD），不是精确的 UTF-16 码元长度；仅作快速预检。
 */
bool is_path_too_long(std::string_view path);

/**
 * @brief 判断文件是否存在且为普通文件
 * @param path 文件路径
 * @return 存在且为文件返回true，否则返回false
 */
bool file_exists(std::string_view path);

/**
 * @brief 创建空文件
 * @param path 要创建的文件路径
 * @return 创建成功返回true，否则返回false
 * @note 若文件已存在，将被截断为0字节
 */
bool create_file(std::string_view path);

/**
 * @brief 删除文件
 * @param path 要删除的文件路径
 * @return 删除成功返回true，否则返回false
 */
bool delete_file(std::string_view path);

/**
 * @brief 复制文件
 * @param src 源文件路径
 * @param dest 目标文件路径
 * @param overwrite 若目标已存在，是否覆盖（默认为false）
 * @return 复制成功返回true，否则返回false
 */
bool copy_file(std::string_view src, std::string_view dest, bool overwrite = false);

/**
 * @brief 移动文件
 * @param src 源文件路径
 * @param dest 目标文件路径
 * @param overwrite 若目标已存在，是否覆盖（默认为false）
 * @return 复制成功返回true，否则返回false
 */
bool rename_file(std::string_view src, std::string_view dest, bool overwrite = false);

/**
 * @brief 读取文本文件内容
 * @param path 文本文件路径
 * @return 文件内容字符串，读取失败返回空字符串
 */
std::string read_text_file(std::string_view path);

/**
 * @brief 读取二进制文件内容
 * @param path 二进制文件路径
 * @return 二进制数据容器，读取失败返回空容器
 */
std::vector<uint8_t> read_binary_file(std::string_view path);

/**
 * @brief 写入文本内容到文件
 * @param path 目标文件路径
 * @param content 要写入的文本内容
 * @param overwrite 是否以覆盖模式写入（默认为false，追加模式）
 * @return 写入成功返回true，否则返回false
 */
bool write_text_file(std::string_view path, std::string_view content, bool overwrite = false);

/**
 * @brief 获取文件大小（字节数）
 * @param path 文件路径
 * @return 文件大小（字节），获取失败返回0
 */
FileSize get_file_size(std::string_view path);

TimestampMs get_file_modify_time(std::string_view path);

/**
 * @brief 获取文件的详细信息
 * @param path 文件路径
 * @return 包含文件信息的FileInfo结构体，获取失败返回默认初始化的结构体
 */
FileInfo get_file_info(std::string_view path);

/**
 * @brief 判断目录是否存在且为目录
 * @param path 目录路径
 * @return 存在且为目录返回true，否则返回false
 */
bool dir_exists(std::string_view path);

/**
 * @brief 创建目录
 * @param path 要创建的目录路径
 * @param recursive 是否递归创建父目录（默认为false）
 * @return 创建成功返回true，否则返回false
 */
bool create_dir(std::string_view path, bool recursive = true);

/**
 * @brief 删除目录
 * @param path 要删除的目录路径
 * @param recursive 是否递归删除非空目录（默认为false）
 * @return 删除成功返回true，否则返回false
 */
bool delete_dir(std::string_view path, bool recursive = true);

}  // namespace origin::filesystem
#endif  // ORIGIN_FILESYSTEM_FILESYSTEM_H
