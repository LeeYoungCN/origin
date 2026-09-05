#include "internal/utils/filesystem_utils_internal.h"

#include "common/compiler/macros.h"
#include "common/types/error_code_types.h"
#include "utils/utils_error_code.h"

#if COMPILER_MSVC
#include <windows.h>
#else
#include <cerrno>
#endif

#include <exception>
#include <filesystem>
#include <system_error>
#include <unordered_map>

#include "common/debug/debug_logger.h"
#include "common/types/error_code_types.h"
#include "internal/utils/filesystem_utils_internal.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

namespace origin::filesystem::internal {

ErrorCode ConvertGenericCategory(const std::error_code& ec)
{
    static const std::unordered_map<int, ErrorCode> ERR_MAP = {
        {static_cast<int>(std::errc::permission_denied), ERR_UTILS_PERMISSION_DENIED},
        {static_cast<int>(std::errc::no_such_file_or_directory), ERR_UTILS_NOT_FOUND},
        {static_cast<int>(std::errc::file_exists), ERR_UTILS_ALREADY_EXISTS},
        {static_cast<int>(std::errc::directory_not_empty), ERR_UTILS_DIR_NOT_EMPTY},
        {static_cast<int>(std::errc::is_a_directory), ERR_UTILS_IS_A_DIRECTORY},
        {static_cast<int>(std::errc::filename_too_long), ERR_UTILS_PATH_TOO_LONG},
        {static_cast<int>(std::errc::invalid_argument), ERR_UTILS_PATH_INVALID},
        {static_cast<int>(std::errc::io_error), ERR_UTILS_IO_ERROR}};

    auto it = ERR_MAP.find(ec.value());
    return (it != ERR_MAP.end() ? it->second : ERR_COMM_GENERIC_ERROR);
}

ErrorCode ConvertSystemCategory(const std::error_code& ec)
{
    static const std::unordered_map<int, ErrorCode> ERR_MAP = {
#if COMPILER_MSVC
        {ERROR_FILE_NOT_FOUND, ERR_UTILS_NOT_FOUND},
        {ERROR_DIR_NOT_EMPTY, ERR_UTILS_DIR_NOT_EMPTY},
        {ERROR_PATH_NOT_FOUND, ERR_UTILS_NOT_FOUND},
        {ERROR_FILE_EXISTS, ERR_UTILS_ALREADY_EXISTS},
        {ERROR_SHARING_VIOLATION, ERR_UTILS_SHARING_VIOLATION},
        {ERROR_ACCESS_DENIED, ERR_UTILS_PERMISSION_DENIED}
#else
        {EIO, ERR_UTILS_IO_ERROR}, {EBUSY, ERR_UTILS_SHARING_VIOLATION}
#endif
    };

    auto it = ERR_MAP.find(ec.value());
    return (it != ERR_MAP.end() ? it->second : ERR_COMM_SYSTEM_ERROR);
}

ErrorCode ConvertExceptionToErrorCode(const std::exception& ex)
{
    try {
        std::throw_with_nested(ex);
    } catch (const std::filesystem::filesystem_error& fse) {
        ORIGIN_DEBUG_WARN("File system error: {} (code: {})", fse.what(), fse.code().value());
        return ConvertSysEcToErrorCode(fse.code());
    } catch (const std::system_error& se) {
        ORIGIN_DEBUG_WARN("System error: {} (code: {})", se.what(), se.code().value());
        return ConvertSysEcToErrorCode(se.code());
    } catch (const std::exception& other) {
        ORIGIN_DEBUG_ERR("Non-filesystem exception, ex: {}.", other.what());
        return ERR_COMM_SYSTEM_ERROR;
    }
}

ErrorCode ConvertSysEcToErrorCode(const std::error_code& ec)
{
    if (!ec) {
        set_thread_last_err(ERR_COMM_SUCCESS);
        return ERR_COMM_SUCCESS;
    }
    // 详细日志便于调试
    ORIGIN_DEBUG_WARN("Convert error: {} (category: {}, value: {})",
                      ec.message().c_str(),
                      ec.category().name(),
                      ec.value());

    if (ec.category() == std::system_category()) {
        return ConvertSystemCategory(ec);
    } else if (ec.category() == std::generic_category()) {
        return ConvertGenericCategory(ec);
    } else {
        ORIGIN_DEBUG_FATAL("Unknown error category: {}", ec.category().name());
        return ERR_COMM_UNKNOWN_ERROR;
    }
}

}  // namespace origin::filesystem::internal
