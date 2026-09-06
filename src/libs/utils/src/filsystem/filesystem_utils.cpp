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
#include "utils/filesystem_utils.h"

#include "common/macros/compiler.h"

#if OS_WINDOWS
#include <windows.h>
#elif OS_LINUX
#include <unistd.h>  // Linux的readlink函数
#elif OS_MACOS
#include <mach-o/dyld.h>  // macOS的_NSGetExecutablePath
#endif

#include <filesystem>
#include <system_error>

#include "common/debug/debug_logger.h"
#include "common/types/filesystem_types.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

namespace origin::filesystem {
namespace fs = std::filesystem;

EntryType get_entry_type(std::string_view path)
{
    if (!fs::exists(path)) {
        set_thread_last_err(ERR_UTILS_NOT_FOUND);
        return EntryType::NONEXISTENT;
    }
    // 获取文件状态（使用symlink_status而非status，保留符号链接本身的类型）
    std::error_code ec;  // 用于非抛出式错误处理
    fs::file_status status = fs::symlink_status(path, ec);
    if (ec) {
        ORIGIN_DEBUG_ERR("Failed to get symlink status. errCode: {}", ec.value());
        set_thread_last_err(ERR_COMM_SYSTEM_ERROR);
        return EntryType::UNKNOWN;
    }
    set_thread_last_err(ERR_COMM_SUCCESS);
    switch (status.type()) {
        case fs::file_type::regular:  // 普通文件
            return EntryType::FILE;
        case fs::file_type::directory:
            return EntryType::DIRECTORY;
        case fs::file_type::symlink:
            return EntryType::SYMBOL_LINK;
        case fs::file_type::character:  // 字符设备
            return EntryType::CHARACTER_DEVICE;
        case fs::file_type::block:
            return EntryType::BLOCK_DEVICE;
        case fs::file_type::fifo:
            return EntryType::SOCKET;
        case fs::file_type::unknown:
        default:
            ORIGIN_DEBUG_WARN("Unknown entry type: {}", path);
            return EntryType::UNKNOWN;
    }
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
