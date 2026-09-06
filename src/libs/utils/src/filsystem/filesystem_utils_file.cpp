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

#include "common/macros/compiler.h"

#if OS_WINDOWS
#include <windows.h>

#include "common/constants/date_time_constants.h"
#else
#include <chrono>
#endif

#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include "common/debug/debug_logger.h"
#include "common/types/date_time_types.h"
#include "common/types/filesystem_types.h"
#include "internal/utils/filesystem_utils_internal.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

// std::ios::out：默认模式，创建文件（若存在则截断）。
// std::ios::app：追加模式（文件存在时在末尾添加内容，不截断）。
// std::ios::binary：二进制模式（避免文本模式的换行符转换）。
// std::ios::trunc 表示如果文件存在则截断它（覆盖原有内容）

namespace {
namespace fs = std::filesystem;
using namespace origin::filesystem;
using namespace origin::filesystem::internal;
#if OS_WINDOWS
using namespace origin::date_time;
#endif  // OS_WINDOWS

bool delete_file_it(std::string_view path)
{
    try {
        bool result = fs::remove(path);
        set_thread_last_err(result ? ERR_COMM_SUCCESS : ERR_UTILS_NOT_FOUND);
        ORIGIN_DEBUG_TRACE(
            "Delete file success. file: \"{}\". msg: \"{}\".", path, get_thread_last_err_msg());
        return result;
    } catch (const fs::filesystem_error& e) {
        ORIGIN_DEBUG_ERR("Delete file failed. file: \"{}\". msg: \"{}\".", path, e.what());
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        return false;
    } catch (const std::exception& e) {
        ORIGIN_DEBUG_ERR("Delete file failed. file: \"{}\". msg: \"{}\".", path, e.what());
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        return false;
    }
}
}  // namespace

namespace origin::filesystem {

namespace fs = std::filesystem;
using namespace origin::filesystem::internal;

bool file_exists(std::string_view path)
{
    auto entrType = get_entry_type(path);
    bool exists = false;
    switch (entrType) {
        case EntryType::FILE:
            exists = true;
            set_thread_last_err(ERR_COMM_SUCCESS);
            break;
        case EntryType::NONEXISTENT:
            set_thread_last_err(ERR_UTILS_NOT_FOUND);
            break;
        default:
            set_thread_last_err(ERR_UTILS_NOT_FILE);
            break;
    }
    return exists;
}

bool create_file(std::string_view path)
{
    EntryType type = get_entry_type(path);
    if (type == EntryType::FILE) {
        set_thread_last_err(ERR_UTILS_ALREADY_EXISTS);
        ORIGIN_DEBUG_TRACE("File already exist: {}", path);
        return true;
    }
    if (type != EntryType::NONEXISTENT) {
        set_thread_last_err(ERR_UTILS_NOT_FILE);
        ORIGIN_DEBUG_ERR(
            "Create file failed. file: \"{}\". msg: \"{}\".", path, get_thread_last_err_msg());
        return false;
    }

    std::ofstream file(path.data(), std::ios::app | std::ios::binary);
    if (file.is_open()) {
        set_thread_last_err(ERR_COMM_SUCCESS);
        return true;
    }
    std::error_code ec(errno, std::generic_category());
    set_thread_last_err(ConvertSysEcToErrorCode(ec));
    ORIGIN_DEBUG_ERR(
        "Create file failed. file: \"{}\". msg: \"{}\".", path, get_thread_last_err_msg());
    return false;
}

bool delete_file(std::string_view path)
{
    if (!file_exists(path)) {
        bool rst = (get_thread_last_err() == ERR_UTILS_NOT_FOUND);
        if (get_thread_last_err() == ERR_UTILS_NOT_FOUND) {
            ORIGIN_DEBUG_TRACE(
                "Delete file success. file: \"{}\". msg: \"{}\".", path, get_thread_last_err_msg());
        } else {
            ORIGIN_DEBUG_ERR(
                "Delete file failed. file: \"{}\". msg: \"{}\".", path, get_thread_last_err_msg());
        }
        return rst;
    }

    return delete_file_it(path);
}

bool copy_file(std::string_view src, std::string_view dest, bool overwrite)
{
    if (!file_exists(src)) {
        ORIGIN_DEBUG_ERR(
            "Copy file failed. file: {}, msg: \"{}\".", src, get_thread_last_err_msg());
        return false;
    }

    EntryType type = get_entry_type(dest);
    type = get_entry_type(dest);
    if (type != EntryType::FILE && type != EntryType::NONEXISTENT) {
        ORIGIN_DEBUG_ERR("Copy file failed. file: {}, dest invalid: {}, type: {}",
                         src,
                         dest,
                         get_entry_type_str(type));
        set_thread_last_err(ERR_UTILS_NOT_FILE);
        return false;
    }
    fs::copy_options option =
        (overwrite ? fs::copy_options::overwrite_existing : fs::copy_options::none);
    try {
        fs::copy_file(src, dest, option);
        set_thread_last_err(ERR_COMM_SUCCESS);
        return true;
    } catch (const fs::filesystem_error& e) {
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Copy file failed. file: {}, msg: \"{}\".", src, e.what());
        return false;
    } catch (const std::exception& e) {
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Copy file failed. file: {}, msg: \"{}\".", src, e.what());
        return false;
    }
}

bool rename_file(std::string_view src, std::string_view dest, bool overwrite)
{
    if (!file_exists(src)) {
        ORIGIN_DEBUG_ERR(
            "Rename file failed. file: {}, msg: \"{}\".", src, get_thread_last_err_msg());
        return false;
    }

    EntryType type = get_entry_type(dest);
    // 类型错误
    if (type != EntryType::FILE && type != EntryType::NONEXISTENT) {
        set_thread_last_err(ERR_UTILS_NOT_FILE);
        ORIGIN_DEBUG_ERR("Rename file failed. file: {}. dest invalid: {}, type: {}",
                         (overwrite ? "overwrite " : "not overwrite"),
                         dest,
                         get_entry_type_str(type));
        return false;
    }
    // 已存在
    if (!overwrite && type == EntryType::FILE) {
        set_thread_last_err(ERR_UTILS_ALREADY_EXISTS);
        ORIGIN_DEBUG_ERR("Rename file failed. file: {}. dest already exist: {}.",
                         (overwrite ? "overwrite " : "not overwrite"),
                         dest);
        return false;
    }

    try {
        fs::rename(src, dest);
        set_thread_last_err(ERR_COMM_SUCCESS);
        return true;
    } catch (const fs::filesystem_error& e) {
        set_thread_last_err(ConvertSysEcToErrorCode(e.code()));
        ORIGIN_DEBUG_ERR("Rename file failed. file: {}, msg: \"{}\".", src, e.what());
        return false;
    } catch (const std::exception& e) {
        set_thread_last_err(ConvertExceptionToErrorCode(e));
        ORIGIN_DEBUG_ERR("Rename file failed. file: {}, msg: \"{}\".", src, e.what());
        return false;
    }
}

std::string read_text_file(std::string_view path)
{
    if (!file_exists(path)) {
        ORIGIN_DEBUG_ERR(
            "Read file failed. file: \"{}\". msg: \"{}\".", path, get_thread_last_err_msg());
        return "";
    }

    std::ifstream file(path.data(), std::ios::in);
    if (!file.is_open()) {
        std::error_code ec(errno, std::generic_category());
        set_thread_last_err(ConvertSysEcToErrorCode(ec));
        ORIGIN_DEBUG_ERR(
            "Read file failed. file: \"{}\". msg: \"{}\".", path, get_thread_last_err_msg());
        return "";
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

std::vector<uint8_t> read_binary_file(std::string_view path)
{
    if (!file_exists(path)) {
        ORIGIN_DEBUG_ERR(
            "Read binary file failed. file: \"{}\". msg: \"{}\".", path, get_thread_last_err_msg());
        return {};
    }

    std::ifstream file(path.data(), std::ios::in);
    if (!file.is_open()) {
        std::error_code ec(errno, std::generic_category());
        set_thread_last_err(ConvertSysEcToErrorCode(ec));
        ORIGIN_DEBUG_ERR(
            "Read binary file failed. file: \"{}\". msg: \"{}\".", path, get_thread_last_err_msg());
        return {};
    }
    FileSize fileSize = fs::file_size(path);
    std::vector<uint8_t> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<int64_t>(fileSize));
    if (static_cast<size_t>(file.gcount()) != fileSize) {
        return {};
    }
    return buffer;
}

bool write_text_file(std::string_view path, std::string_view content, bool overwrite)
{
    if (!file_exists(path)) {
        ORIGIN_DEBUG_ERR("Write to text file failed. file: \"{}\". msg: \"{}\".",
                         path,
                         get_thread_last_err_msg());
        return false;
    }

    std::ios::openmode mode = std::ios::out;
    if (overwrite) {
        mode = (std::ios::out | std::ios::trunc);
    } else {
        mode = (std::ios::out | std::ios::app);
    }
    std::ofstream file(path.data(), mode);
    if (!file.is_open()) {
        std::error_code ec(errno, std::generic_category());
        set_thread_last_err(ConvertSysEcToErrorCode(ec));
        ORIGIN_DEBUG_ERR("Write to text file failed. file: \"{}\". msg: \"{}\".",
                         path,
                         get_thread_last_err_msg());
        return false;
    }

    if (file << content) {
        file.close();
        set_thread_last_err(ERR_COMM_SUCCESS);
        ORIGIN_DEBUG_TRACE("Write to text file succeeded. file: \"{}\". msg: \"{}\".",
                           get_file_mode_str(overwrite),
                           path,
                           get_thread_last_err_msg());
        return true;
    }

    std::error_code ec(errno, std::system_category());
    set_thread_last_err(ConvertSysEcToErrorCode(ec));

    ORIGIN_DEBUG_ERR("Write text failed. file {}: {}, msg: \"{}\".",
                     get_file_mode_str(overwrite),
                     path,
                     get_thread_last_err_msg());
    return false;
}

FileSize get_file_size(std::string_view path)
{
    if (!file_exists(path)) {
        ORIGIN_DEBUG_ERR(
            "Get file size failed. file: \"{}\", msg: \"{}\".", path, get_thread_last_err_msg());
        return 0;
    }
    std::error_code ec;
    const auto& size = fs::file_size(path, ec);
    set_thread_last_err(ConvertSysEcToErrorCode(ec));
    return size;
}

TimestampMs get_file_modify_time(std::string_view path)
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
    if (!ok) return 0;

    FILETIME ftCreate = fileAttr.ftCreationTime;
    ULARGE_INTEGER ul;
    ul.LowPart = ftCreate.dwLowDateTime;
    ul.HighPart = ftCreate.dwHighDateTime;

    // FILETIME基准：1601-01-01 单位100ns
    // 转毫秒，再减去1601到1970的毫秒差值 11644473600000
    int64_t totalMs = static_cast<int64_t>(ul.QuadPart) / HUNDRED_NANOSECONDS_PER_MILLISECOND;
    return totalMs - WINDOWS_EPOCH_TO_UNIX_EPOCH_MS;
#else
    fs::file_time_type fileTime = fs::last_write_time(path);
    auto sysTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        fileTime - std::filesystem::file_time_type::clock::now() +
        std::chrono::system_clock::now());
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(sysTime.time_since_epoch());
    return static_cast<TimestampMs>(ms.count());
#endif
}

FileInfo get_file_info(std::string_view path)
{
    FileInfo fileInfo{};
    fileInfo.path = path;
    fileInfo.type = get_entry_type(path);

    if (fileInfo.type == EntryType::NONEXISTENT) {
        return fileInfo;
    }
    if (fileInfo.type == EntryType::FILE) {
        fileInfo.size = fs::file_size(path);
    }
    fileInfo.modifyTime = get_file_modify_time(path);

    return fileInfo;
}

}  // namespace origin::filesystem
