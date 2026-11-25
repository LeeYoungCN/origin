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
 * 2. 错误处理统一：不向调用方抛异常，可失败接口统一返回 `ErrorCode`，
 *    需要产出值的用出参；错误文本用 `get_utils_err_msg(code)` 获取。
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
 * - 失败返回错误码（`ErrorCode`）或空字符串 / 0，具体见各接口说明。
 */
#ifndef ORIGIN_FILESYSTEM_FILESYSTEM_H
#define ORIGIN_FILESYSTEM_FILESYSTEM_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "origin/common/types/date_time_types.h"
#include "origin/common/types/error_code_types.h"
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
[[nodiscard]] ErrorCode get_entry_type(std::string_view path, EntryType& out);

const char* get_entry_type_str(EntryType type);

inline const char* get_file_mode_str(bool overwrite)
{
    return overwrite ? "overwrite" : "append";
}

/**
 * @brief 获取当前工作路径。
 *
 * @param[out] out 当前工作路径；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode get_curr_working_dir(std::string& out);

/**
 * @brief 拼接多个路径片段为完整路径
 * @param parts 路径片段列表
 * @param[out] out 拼接后的完整路径字符串；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS；列表为空返回 ERR_UTILS_PATH_INVALID
 * @note 自动处理不同平台的路径分隔符
 * @note 片段中若出现绝对路径，会替换掉前面已拼接的部分
 *       （std::filesystem::operator/= 语义）
 */
[[nodiscard]] ErrorCode join_paths(const std::vector<std::string>& parts, std::string& out);

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
 * @param[out] out 规范化后的路径；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 * @see to_absolute_path 需要绝对路径时使用
 */
[[nodiscard]] ErrorCode normalize_path(std::string_view path, std::string& out);

/**
 * @brief 将路径拼到基准目录后转为绝对路径
 *
 * 过程：base = baseDir（为空时取 get_curr_working_dir()）→ base / relPath
 * → std::filesystem::absolute → lexically_normal。
 *
 * - relPath 为空时直接返回 ERR_COMM_SUCCESS，并将 out 置为空串；
 * - relPath 为绝对路径时忽略 base（operator/= 语义）；
 * - 不检查路径是否存在，不解析符号链接；
 * - baseDir 为相对路径时按当前工作目录解析；
 * - 结果中的 `.`、`..` 已按词法折叠。
 *
 * @param relPath 相对路径（也允许传绝对路径）
 * @param baseDir 基准目录（传空串表示使用当前工作目录）
 * @param[out] out 绝对路径；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 * @see normalize_path 只做词法规范化、不转绝对路径
 */
[[nodiscard]] ErrorCode to_absolute_path(std::string_view relPath,
                                         std::string_view baseDir,
                                         std::string& out);

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
 * @note 该接口只返回布尔值：权限不足、路径过长等错误也会返回false。
 *       需要区分“不存在”和“无法访问”时，请使用 get_entry_type。
 */
bool file_exists(std::string_view path);

/**
 * @brief 创建空文件
 * @param path 要创建的文件路径
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 * @note 若文件已存在，直接返回成功，不会截断或修改内容。
 */
[[nodiscard]] ErrorCode create_file(std::string_view path);

/**
 * @brief 删除文件
 * @param path 要删除的文件路径
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode delete_file(std::string_view path);

/**
 * @brief 复制文件
 * @param src 源文件路径
 * @param dest 目标文件路径
 * @param overwrite 若目标已存在，是否覆盖（默认为false）
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode copy_file(std::string_view src,
                                  std::string_view dest,
                                  bool overwrite = false);

/**
 * @brief 移动文件
 * @param src 源文件路径
 * @param dest 目标文件路径
 * @param overwrite 若目标已存在，是否覆盖（默认为false）
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode rename_file(std::string_view src,
                                    std::string_view dest,
                                    bool overwrite = false);

/**
 * @brief 读取文本文件内容
 * @param path 文本文件路径
 * @param[out] out 文件内容；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS；文件不存在返回 ERR_UTILS_NOT_FOUND，
 *         存在但不是文件返回 ERR_UTILS_NOT_FILE
 */
[[nodiscard]] ErrorCode read_text_file(std::string_view path, std::string& out);

/**
 * @brief 读取二进制文件内容
 * @param path 二进制文件路径
 * @param[out] out 文件内容；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS；文件不存在返回 ERR_UTILS_NOT_FOUND，
 *         存在但不是文件返回 ERR_UTILS_NOT_FILE
 */
[[nodiscard]] ErrorCode read_binary_file(std::string_view path, std::vector<uint8_t>& out);

/**
 * @brief 写入文本内容到文件
 * @param path 目标文件路径
 * @param content 要写入的文本内容
 * @param overwrite 是否以覆盖模式写入（默认为false，追加模式）
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode write_text_file(std::string_view path,
                                        std::string_view content,
                                        bool overwrite = false);

/**
 * @brief 获取文件大小（字节数）
 * @param path 文件路径
 * @param[out] out 文件大小（字节）；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS；文件不存在返回 ERR_UTILS_NOT_FOUND，
 *         存在但不是文件返回 ERR_UTILS_NOT_FILE
 */
[[nodiscard]] ErrorCode get_file_size(std::string_view path, FileSize& out);

/**
 * @brief 获取文件的最后修改时间
 * @param path 文件路径
 * @param[out] out 毫秒级时间戳（Unix纪元基准）；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode get_file_modify_time(std::string_view path, TimestampMs& out);

/**
 * @brief 获取文件的详细信息
 * @param path 文件路径
 * @param[out] out 文件信息（路径/类型/大小/修改时间）；失败时 path 与 type
 *                 仍会写入，size 与 modifyTime 保持默认值
 * @return 成功返回 ERR_COMM_SUCCESS；路径不存在返回 ERR_UTILS_NOT_FOUND
 */
[[nodiscard]] ErrorCode get_file_info(std::string_view path, FileInfo& out);

/**
 * @brief 判断目录是否存在且为目录
 * @param path 目录路径
 * @return 存在且为目录返回true，否则返回false
 * @note 该接口只返回布尔值：权限不足、路径过长等错误也会返回false。
 *       需要区分“不存在”和“无法访问”时，请使用 get_entry_type。
 */
bool dir_exists(std::string_view path);

/**
 * @brief 创建目录
 * @param path 要创建的目录路径
 * @param recursive 是否递归创建父目录（默认为true）
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode create_dir(std::string_view path, bool recursive = true);

/**
 * @brief 删除目录
 * @param path 要删除的目录路径
 * @param recursive 是否递归删除非空目录（默认为true）
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode delete_dir(std::string_view path, bool recursive = true);

// ------------------------------ 值返回便捷重载（忽略错误码） ------------------------------

/**
 * @brief 获取路径条目类型（值返回）
 * @note 路径不存在返回 EntryType::NONEXISTENT；获取失败返回 EntryType::UNKNOWN
 */
inline EntryType get_entry_type(std::string_view path)
{
    EntryType out = EntryType::UNKNOWN;
    (void)get_entry_type(path, out);
    return out;
}

/**
 * @brief 获取当前工作路径（值返回）
 * @return 当前工作路径；失败时返回空串
 */
inline std::string get_curr_working_dir()
{
    std::string out;
    (void)get_curr_working_dir(out);
    return out;
}

/**
 * @brief 拼接多个路径片段（值返回）
 * @return 拼接后的路径；空列表或失败时返回空串
 */
inline std::string join_paths(const std::vector<std::string>& parts)
{
    std::string out;
    (void)join_paths(parts, out);
    return out;
}

/**
 * @brief 纯词法规范化路径（值返回）
 * @return 规范化后的路径；失败时返回空串
 */
inline std::string normalize_path(std::string_view path)
{
    std::string out;
    (void)normalize_path(path, out);
    return out;
}

/**
 * @brief 将路径拼到基准目录后转为绝对路径（值返回）
 * @param relPath 相对路径（也允许传绝对路径）
 * @param baseDir 基准目录，默认空串表示当前工作目录
 * @return 绝对路径；失败时返回空串
 */
inline std::string to_absolute_path(std::string_view relPath, std::string_view baseDir = "")
{
    std::string out;
    (void)to_absolute_path(relPath, baseDir, out);
    return out;
}

/**
 * @brief 读取文本文件内容（值返回）
 * @return 文件内容；文件不存在/读取失败时返回空串
 */
inline std::string read_text_file(std::string_view path)
{
    std::string out;
    (void)read_text_file(path, out);
    return out;
}

/**
 * @brief 读取二进制文件内容（值返回）
 * @return 文件内容；文件不存在/读取失败时返回空容器
 */
inline std::vector<uint8_t> read_binary_file(std::string_view path)
{
    std::vector<uint8_t> out;
    (void)read_binary_file(path, out);
    return out;
}

/**
 * @brief 获取文件大小（值返回）
 * @return 文件大小（字节）；失败时返回 0
 */
inline FileSize get_file_size(std::string_view path)
{
    FileSize out = 0;
    (void)get_file_size(path, out);
    return out;
}

/**
 * @brief 获取文件的最后修改时间（值返回）
 * @return 毫秒级时间戳；失败时返回 0
 */
inline TimestampMs get_file_modify_time(std::string_view path)
{
    TimestampMs out = 0;
    (void)get_file_modify_time(path, out);
    return out;
}

/**
 * @brief 获取文件的详细信息（值返回）
 * @return 文件信息；路径不存在时 type 为 NONEXISTENT，size/modifyTime 为 0
 */
inline FileInfo get_file_info(std::string_view path)
{
    FileInfo out{};
    (void)get_file_info(path, out);
    return out;
}

}  // namespace origin::filesystem
#endif  // ORIGIN_FILESYSTEM_FILESYSTEM_H
