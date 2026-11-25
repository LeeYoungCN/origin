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

#include <filesystem>
#include <string_view>
#include <system_error>

#include "origin/common/common_error_code.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/common/types/error_code_types.h"
#include "origin/common/types/filesystem_types.h"

#include "origin/filesystem/filesystem.h"
#include "origin/utils/utils_error_code.h"

#include "filesystem/internal/common.hpp"

namespace origin::filesystem {
namespace fs = std::filesystem;

ErrorCode get_entry_type(std::string_view path, EntryType& out)
{
    // symlink_status 不跟随链接，可保留“链接本身”的类型；用 error_code 重载避免抛异常。
    std::error_code ec;
    fs::file_status const status = fs::symlink_status(path, ec);
    if (ec) {
        // 路径中间某段不是目录时（如 "<file>/missing"），symlink_status 返回
        // ENOTDIR/ERROR_DIRECTORY；该路径不可能对应任何目录项，应按“不存在”处理，
        // 与 std::filesystem::exists 的语义及 remove/delete_dir 的幂等行为保持一致。
        if (ec == std::errc::no_such_file_or_directory || ec == std::errc::not_a_directory) {
            out = EntryType::NONEXISTENT;
            return ERR_UTILS_NOT_FOUND;
        }
        ORIGIN_DEBUG_ERR("Failed to get symlink status. errCode: {}", ec.value());
        out = EntryType::UNKNOWN;
        return ConvertSysEcToErrorCode(ec);
    }
    switch (status.type()) {
        case fs::file_type::not_found:
            out = EntryType::NONEXISTENT;
            return ERR_UTILS_NOT_FOUND;
        case fs::file_type::regular:  // 普通文件
            out = EntryType::FILE;
            break;
        case fs::file_type::directory:
            out = EntryType::DIRECTORY;
            break;
        case fs::file_type::symlink:
            out = EntryType::SYMBOL_LINK;
            break;
        case fs::file_type::character:  // 字符设备
            out = EntryType::CHARACTER_DEVICE;
            break;
        case fs::file_type::block:
            out = EntryType::BLOCK_DEVICE;
            break;
        case fs::file_type::fifo:
            out = EntryType::FIFO;
            break;
        case fs::file_type::socket:
            out = EntryType::SOCKET;
            break;
        case fs::file_type::unknown:
        default:
            ORIGIN_DEBUG_WARN("Unknown entry type: {}", path);
            out = EntryType::UNKNOWN;
            break;
    }
    return ERR_COMM_SUCCESS;
}

const char* get_entry_type_str(EntryType type)
{
    switch (type) {
        case EntryType::NONEXISTENT:
            return "Nonexistent";
        case EntryType::FILE:
            return "File";
        case EntryType::DIRECTORY:
            return "Directory";
        case EntryType::SYMBOL_LINK:
            return "Symbol link";
        case EntryType::BLOCK_DEVICE:
            return "Block device";
        case EntryType::CHARACTER_DEVICE:
            return "Character Device";
        case EntryType::FIFO:
            return "FIFO";
        case EntryType::SOCKET:
            return "Socket";
        case EntryType::UNKNOWN:
        default:
            return "Unknown";
    }
}

}  // namespace origin::filesystem
