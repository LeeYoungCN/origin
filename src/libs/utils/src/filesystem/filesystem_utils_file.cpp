/**
 * @file filesystem_utils.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-30
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "origin/common/macros/compiler.h"

#if OS_WINDOWS
// NOLINTNEXTLINE(misc-include-cleaner): SDK 窄头文件的平台宏由 windows.h 建立
#include <windows.h>

#include <errhandlingapi.h>
#include <fileapi.h>
#include <minwinbase.h>
#include <minwindef.h>
#include <stringapiset.h>
#include <winnls.h>
#include <winnt.h>

#include "origin/common/constants/date_time_constants.h"
#else
#include <chrono>
#endif

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "filesystem/internal/common.hpp"
#include "origin/common/common_error_code.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/common/types/date_time_types.h"
#include "origin/common/types/error_code_types.h"
#include "origin/common/types/filesystem_types.h"
#include "origin/filesystem/filesystem.h"
#include "origin/utils/utils_error_code.h"

// std::ios::out：默认模式，创建文件（若存在则截断）。
// std::ios::app：追加模式（文件存在时在末尾添加内容，不截断）。
// std::ios::binary：二进制模式（避免文本模式的换行符转换）。
// std::ios::trunc 表示如果文件存在则截断它（覆盖原有内容）

namespace {
namespace fs = std::filesystem;
using namespace origin::filesystem;
#if OS_WINDOWS
using namespace origin::date_time;
#endif  // OS_WINDOWS

// 确认 path 为普通文件：不存在返回 ERR_UTILS_NOT_FOUND，存在但不是普通文件
// 返回 ERR_UTILS_NOT_FILE，其余失败返回底层错误码。
ErrorCode check_is_file(std::string_view path)
{
    EntryType type = EntryType::UNKNOWN;
    const ErrorCode code = get_entry_type(path, type);
    if (code != ERR_COMM_SUCCESS) {
        return code;
    }
    return type == EntryType::FILE ? ERR_COMM_SUCCESS : ERR_UTILS_NOT_FILE;
}

ErrorCode delete_file_it(std::string_view path)
{
    try {
        if (!fs::remove(path)) {
            ORIGIN_DEBUG_ERR("Delete file failed. file: \"{}\". msg: \"{}\".",
                             path,
                             get_utils_err_msg(ERR_UTILS_NOT_FOUND));
            return ERR_UTILS_NOT_FOUND;
        }
        ORIGIN_DEBUG_TRACE("Delete file success. file: \"{}\".", path);
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR("Delete file failed. file: \"{}\". msg: \"{}\".", path, e.what());
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR("Delete file failed. file: \"{}\". msg: \"{}\".", path, e.what());
        return code;
    }
}
}  // namespace

namespace origin::filesystem {

namespace fs = std::filesystem;

bool file_exists(std::string_view path)
{
    EntryType entrType = EntryType::UNKNOWN;
    (void)get_entry_type(path, entrType);
    return entrType == EntryType::FILE;
}

ErrorCode create_file(std::string_view path)
{
    EntryType type = EntryType::UNKNOWN;
    (void)get_entry_type(path, type);
    if (type == EntryType::FILE) {
        ORIGIN_DEBUG_TRACE("File already exist: {}", path);
        return ERR_COMM_SUCCESS;
    }
    if (type != EntryType::NONEXISTENT) {
        ORIGIN_DEBUG_ERR("Create file failed. file: \"{}\". msg: \"{}\".",
                         path,
                         get_utils_err_msg(ERR_UTILS_NOT_FILE));
        return ERR_UTILS_NOT_FILE;
    }

    std::ofstream file(path.data(), std::ios::app | std::ios::binary);
    if (file.is_open()) {
        return ERR_COMM_SUCCESS;
    }
    std::error_code const ec(errno, std::generic_category());
    const ErrorCode code = ConvertSysEcToErrorCode(ec);
    ORIGIN_DEBUG_ERR(
        "Create file failed. file: \"{}\". msg: \"{}\".", path, get_utils_err_msg(code));
    return code;
}

ErrorCode delete_file(std::string_view path)
{
    EntryType type = EntryType::UNKNOWN;
    const ErrorCode typeCode = get_entry_type(path, type);
    if (typeCode != ERR_COMM_SUCCESS && type != EntryType::NONEXISTENT) {
        ORIGIN_DEBUG_ERR(
            "Delete file failed. file: \"{}\". msg: \"{}\".", path, get_utils_err_msg(typeCode));
        return typeCode;
    }
    if (type == EntryType::NONEXISTENT) {
        // 删除不存在的文件视为成功（幂等）。
        ORIGIN_DEBUG_TRACE("Delete file success. file: \"{}\".", path);
        return ERR_COMM_SUCCESS;
    }
    if (type != EntryType::FILE) {
        ORIGIN_DEBUG_ERR("Delete file failed. file: \"{}\". msg: \"{}\".",
                         path,
                         get_utils_err_msg(ERR_UTILS_NOT_FILE));
        return ERR_UTILS_NOT_FILE;
    }
    return delete_file_it(path);
}

ErrorCode copy_file(std::string_view src, std::string_view dest, bool overwrite)
{
    const ErrorCode srcCode = check_is_file(src);
    if (srcCode != ERR_COMM_SUCCESS) {
        ORIGIN_DEBUG_ERR(
            "Copy file failed. file: {}. msg: \"{}\".", src, get_utils_err_msg(srcCode));
        return srcCode;
    }

    EntryType type = EntryType::UNKNOWN;
    (void)get_entry_type(dest, type);
    if (type != EntryType::FILE && type != EntryType::NONEXISTENT) {
        ORIGIN_DEBUG_ERR("Copy file failed. file: {}, dest invalid: {}, type: {}",
                         src,
                         dest,
                         get_entry_type_str(type));
        return ERR_UTILS_NOT_FILE;
    }
    fs::copy_options const option =
        (overwrite ? fs::copy_options::overwrite_existing : fs::copy_options::none);
    try {
        fs::copy_file(src, dest, option);
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR("Copy file failed. file: {}, msg: \"{}\".", src, e.what());
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR("Copy file failed. file: {}, msg: \"{}\".", src, e.what());
        return code;
    }
}

ErrorCode rename_file(std::string_view src, std::string_view dest, bool overwrite)
{
    const ErrorCode srcCode = check_is_file(src);
    if (srcCode != ERR_COMM_SUCCESS) {
        ORIGIN_DEBUG_ERR(
            "Rename file failed. file: {}. msg: \"{}\".", src, get_utils_err_msg(srcCode));
        return srcCode;
    }

    EntryType type = EntryType::UNKNOWN;
    (void)get_entry_type(dest, type);
    // 类型错误
    if (type != EntryType::FILE && type != EntryType::NONEXISTENT) {
        ORIGIN_DEBUG_ERR("Rename file failed. file: {}. dest invalid: {}, type: {}",
                         (overwrite ? "overwrite " : "not overwrite"),
                         dest,
                         get_entry_type_str(type));
        return ERR_UTILS_NOT_FILE;
    }
    // 已存在
    if (!overwrite && type == EntryType::FILE) {
        ORIGIN_DEBUG_ERR("Rename file failed. file: {}. dest already exist: {}.",
                         (overwrite ? "overwrite " : "not overwrite"),
                         dest);
        return ERR_UTILS_ALREADY_EXISTS;
    }

    try {
        fs::rename(src, dest);
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR("Rename file failed. file: {}, msg: \"{}\".", src, e.what());
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR("Rename file failed. file: {}, msg: \"{}\".", src, e.what());
        return code;
    }
}

ErrorCode read_text_file(std::string_view path, std::string& out)
{
    const ErrorCode fileCode = check_is_file(path);
    if (fileCode != ERR_COMM_SUCCESS) {
        ORIGIN_DEBUG_ERR(
            "Read file failed. file: \"{}\". msg: \"{}\".", path, get_utils_err_msg(fileCode));
        return fileCode;
    }

    std::ifstream file(path.data(), std::ios::in);
    if (!file.is_open()) {
        std::error_code const ec(errno, std::generic_category());
        const ErrorCode code = ConvertSysEcToErrorCode(ec);
        ORIGIN_DEBUG_ERR(
            "Read file failed. file: \"{}\". msg: \"{}\".", path, get_utils_err_msg(code));
        return code;
    }

    std::string content{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    out = std::move(content);
    return ERR_COMM_SUCCESS;
}

ErrorCode read_binary_file(std::string_view path, std::vector<uint8_t>& out)
{
    const ErrorCode fileCode = check_is_file(path);
    if (fileCode != ERR_COMM_SUCCESS) {
        ORIGIN_DEBUG_ERR("Read binary file failed. file: \"{}\". msg: \"{}\".",
                         path,
                         get_utils_err_msg(fileCode));
        return fileCode;
    }

    std::ifstream file(path.data(), std::ios::in);
    if (!file.is_open()) {
        std::error_code const ec(errno, std::generic_category());
        const ErrorCode code = ConvertSysEcToErrorCode(ec);
        ORIGIN_DEBUG_ERR(
            "Read binary file failed. file: \"{}\". msg: \"{}\".", path, get_utils_err_msg(code));
        return code;
    }

    std::error_code ec;
    const FileSize fileSize = fs::file_size(path, ec);
    if (ec) {
        const ErrorCode code = ConvertSysEcToErrorCode(ec);
        ORIGIN_DEBUG_ERR(
            "Read binary file failed. file: \"{}\". msg: \"{}\".", path, get_utils_err_msg(code));
        return code;
    }

    std::vector<uint8_t> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<int64_t>(fileSize));
    if (static_cast<size_t>(file.gcount()) != fileSize) {
        ORIGIN_DEBUG_ERR("Read binary file failed. file: \"{}\". msg: \"{}\".",
                         path,
                         get_utils_err_msg(ERR_UTILS_IO_ERROR));
        return ERR_UTILS_IO_ERROR;
    }

    out = std::move(buffer);
    return ERR_COMM_SUCCESS;
}

ErrorCode write_text_file(std::string_view path, std::string_view content, bool overwrite)
{
    const ErrorCode fileCode = check_is_file(path);
    if (fileCode != ERR_COMM_SUCCESS) {
        ORIGIN_DEBUG_ERR("Write to text file failed. file: \"{}\". msg: \"{}\".",
                         path,
                         get_utils_err_msg(fileCode));
        return fileCode;
    }

    std::ios::openmode mode = std::ios::out;
    if (overwrite) {
        mode = (std::ios::out | std::ios::trunc);
    } else {
        mode = (std::ios::out | std::ios::app);
    }
    std::ofstream file(path.data(), mode);
    if (!file.is_open()) {
        std::error_code const ec(errno, std::generic_category());
        const ErrorCode code = ConvertSysEcToErrorCode(ec);
        ORIGIN_DEBUG_ERR(
            "Write to text file failed. file: \"{}\". msg: \"{}\".", path, get_utils_err_msg(code));
        return code;
    }

    if (file << content) {
        file.close();
        ORIGIN_DEBUG_TRACE("Write to text file succeeded. file: \"{}\", mode: {}.",
                           path,
                           get_file_mode_str(overwrite));
        return ERR_COMM_SUCCESS;
    }

    std::error_code const ec(errno, std::system_category());
    const ErrorCode code = ConvertSysEcToErrorCode(ec);
    ORIGIN_DEBUG_ERR("Write text failed. file \"{}\", mode: {}. msg: \"{}\".",
                     path,
                     get_file_mode_str(overwrite),
                     get_utils_err_msg(code));
    return code;
}

ErrorCode get_file_size(std::string_view path, FileSize& out)
{
    const ErrorCode fileCode = check_is_file(path);
    if (fileCode != ERR_COMM_SUCCESS) {
        ORIGIN_DEBUG_ERR(
            "Get file size failed. file: \"{}\". msg: \"{}\".", path, get_utils_err_msg(fileCode));
        return fileCode;
    }

    std::error_code ec;
    const FileSize size = static_cast<FileSize>(fs::file_size(path, ec));
    if (ec) {
        const ErrorCode code = ConvertSysEcToErrorCode(ec);
        ORIGIN_DEBUG_ERR(
            "Get file size failed. file: \"{}\". msg: \"{}\".", path, get_utils_err_msg(code));
        return code;
    }

    out = size;
    return ERR_COMM_SUCCESS;
}

ErrorCode get_file_modify_time(std::string_view path, TimestampMs& out)
{
#if OS_WINDOWS
    WIN32_FILE_ATTRIBUTE_DATA fileAttr{};
    // convert utf-8 path to wide string for Windows API
    std::wstring wpath;
    if (!path.empty()) {
        int sz =
            MultiByteToWideChar(CP_UTF8, 0, path.data(), static_cast<int>(path.size()), nullptr, 0);
        if (sz > 0) {
            wpath.resize(static_cast<size_t>(sz));
            MultiByteToWideChar(
                CP_UTF8, 0, path.data(), static_cast<int>(path.size()), &wpath[0], sz);
        }
    }
    BOOL ok = GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &fileAttr);
    if (!ok) {
        const ErrorCode code = ConvertSysEcToErrorCode(
            std::error_code(static_cast<int>(GetLastError()), std::system_category()));
        ORIGIN_DEBUG_ERR("Get file modify time failed. file: \"{}\", msg: \"{}\".",
                         path,
                         get_utils_err_msg(code));
        return code;
    }

    FILETIME ftCreate = fileAttr.ftCreationTime;
    ULARGE_INTEGER ul;
    ul.LowPart = ftCreate.dwLowDateTime;
    ul.HighPart = ftCreate.dwHighDateTime;

    // FILETIME基准：1601-01-01 单位100ns
    // 转毫秒，再减去1601到1970的毫秒差值 11644473600000
    int64_t totalMs = static_cast<int64_t>(ul.QuadPart) / HUNDRED_NANOSECONDS_PER_MILLISECOND;
    out = totalMs - WINDOWS_EPOCH_TO_UNIX_EPOCH_MS;
    return ERR_COMM_SUCCESS;
#else
    try {
        fs::file_time_type const fileTime = fs::last_write_time(path);
        auto sysTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            fileTime - std::filesystem::file_time_type::clock::now() +
            std::chrono::system_clock::now());
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(sysTime.time_since_epoch());
        out = static_cast<TimestampMs>(ms.count());
        return ERR_COMM_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        const ErrorCode code = ConvertSysEcToErrorCode(e.code());
        ORIGIN_DEBUG_ERR("Get file modify time failed. file: \"{}\", msg: \"{}\".",
                         path,
                         get_utils_err_msg(code));
        return code;
    } catch (const std::exception& e) {
        const ErrorCode code = ConvertExceptionToErrorCode(e);
        ORIGIN_DEBUG_ERR("Get file modify time failed. file: \"{}\", msg: \"{}\".",
                         path,
                         get_utils_err_msg(code));
        return code;
    }
#endif
}

ErrorCode get_file_info(std::string_view path, FileInfo& out)
{
    FileInfo fileInfo{};
    fileInfo.path = path;
    EntryType type = EntryType::UNKNOWN;
    const ErrorCode typeCode = get_entry_type(path, type);
    fileInfo.type = type;

    if (type == EntryType::NONEXISTENT) {
        out = fileInfo;
        return typeCode != ERR_COMM_SUCCESS ? typeCode : ERR_UTILS_NOT_FOUND;
    }
    if (typeCode != ERR_COMM_SUCCESS) {
        out = fileInfo;
        return typeCode;
    }
    if (fileInfo.type == EntryType::FILE) {
        std::error_code ec;
        fileInfo.size = fs::file_size(path, ec);
        if (ec) {
            const ErrorCode code = ConvertSysEcToErrorCode(ec);
            out = fileInfo;
            return code;
        }
    }

    TimestampMs modifyTime = 0;
    const ErrorCode rc = get_file_modify_time(path, modifyTime);
    if (rc != ERR_COMM_SUCCESS) {
        out = fileInfo;
        return rc;
    }
    fileInfo.modifyTime = modifyTime;

    out = fileInfo;
    return ERR_COMM_SUCCESS;
}

}  // namespace origin::filesystem
