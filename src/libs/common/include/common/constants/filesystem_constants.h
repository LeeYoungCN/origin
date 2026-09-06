/**
 * @file filesystem_constants.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-27
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#ifndef COMMON_CONSTANTS_FILESYSTEM_CONSTANTS_H
#define COMMON_CONSTANTS_FILESYSTEM_CONSTANTS_H

#include <cstddef>

#include "common/macros/compiler.h"

namespace origin::filesystem {
/**
 * @brief 路径长度常量（跨平台自动适配）
 */
#if OS_WINDOWS
[[maybe_unused]] constexpr size_t MAX_PATH_STD = 260;     ///< Windows标准路径最大长度（含终止符）
[[maybe_unused]] constexpr size_t MAX_PATH_LONG = 32767;  ///< Windows长路径最大长度（启用后）
#else
[[maybe_unused]] constexpr size_t MAX_PATH_STD = 4096;    ///< Unix系统标准路径最大长度
[[maybe_unused]] constexpr size_t MAX_PATH_LONG = 65536;  ///< Unix系统扩展路径长度
#endif

/**
 * @brief 文件名最大长度（不含路径部分）
 */
#if OS_WINDOWS
[[maybe_unused]] constexpr size_t MAX_FILENAME = 256;  ///< Windows文件名长度限制
#else
[[maybe_unused]] constexpr size_t MAX_FILENAME = 255;  ///< Unix文件名长度限制
#endif

/**
 * @brief 路径分隔符（平台相关）
 */
#if OS_WINDOWS
[[maybe_unused]] constexpr const char* PATH_SEP = "\\";     ///< Windows系统路径分隔符
[[maybe_unused]] constexpr const char* ALT_PATH_SEP = "/";  ///< Windows兼容的替代分隔符
#else
[[maybe_unused]] constexpr const char* PATH_SEP = "/";     ///< Unix系统路径分隔符
[[maybe_unused]] constexpr const char* ALT_PATH_SEP = "";  ///< 无替代分隔符
#endif

#if OS_WINDOWS
[[maybe_unused]] constexpr size_t LF_LENGTH = 2;
#else
[[maybe_unused]] constexpr size_t LF_LENGTH = 1;
#endif

[[maybe_unused]] constexpr unsigned int FILE_OPEN_INTERVAL_MS = 10;

}  // namespace origin::filesystem

#endif  // COMMON_CONSTANTS_FILESYSTEM_CONSTANTS_H
