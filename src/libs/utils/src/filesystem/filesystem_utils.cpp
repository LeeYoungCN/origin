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

#include "origin/utils/utils_error_code.h"

#include "origin/filesystem/filesystem.h"

namespace origin::filesystem {
namespace fs = std::filesystem;

ErrorCode get_entry_type(std::string_view path, EntryType& out)
{
    if (!fs::exists(path)) {
        out = EntryType::NONEXISTENT;
        return ERR_UTILS_NOT_FOUND;
    }
    // 获取文件状态（使用symlink_status而非status，保留符号链接本身的类型）
    std::error_code ec;  // 用于非抛出式错误处理
    fs::file_status const status = fs::symlink_status(path, ec);
    if (ec) {
        ORIGIN_DEBUG_ERR("Failed to get symlink status. errCode: {}", ec.value());
        out = EntryType::UNKNOWN;
        return ERR_COMM_SYSTEM_ERROR;
    }
    switch (status.type()) {
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
