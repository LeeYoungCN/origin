/**
 * @file process.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2026-02-05
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef ORIGIN_PROCESS_PROCESS_H
#define ORIGIN_PROCESS_PROCESS_H

#include <string>

#include "origin/common/types/error_code_types.h"
#include "origin/common/types/process_types.h"

namespace origin::process {

/**
 * @brief 获取当前进程的可执行文件路径
 * @param[out] out 可执行文件的绝对路径；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 * @note 不同平台实现方式不同：
 *       - Windows: 通过GetModuleFileName获取
 *       - Linux/macOS: 通过/proc/self/exe符号链接获取
 */
[[nodiscard]] ErrorCode get_proc_path(std::string& out);

/**
 * @brief 获取当前进程所在目录
 *
 * @param[out] out 目录的绝对路径；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode get_proc_directory(std::string& out);

/**
 * @brief 获取当前进程名称（不含扩展名）
 *
 * @param[out] out 进程文件名；失败时不被修改
 * @return 成功返回 ERR_COMM_SUCCESS，失败返回错误码
 */
[[nodiscard]] ErrorCode get_proc_filename_stem(std::string& out);

/**
 * @brief 获取当前进程ID
 *
 * @return ProcessId 当前进程ID。
 */
ProcessId get_curr_proc_id();

// ------------------------------ 值返回便捷重载（忽略错误码） ------------------------------

/**
 * @brief 获取当前进程的可执行文件路径（值返回）
 * @return 绝对路径；失败时返回空串
 */
inline std::string get_proc_path()
{
    std::string out;
    (void)get_proc_path(out);
    return out;
}

/**
 * @brief 获取当前进程所在目录（值返回）
 * @return 绝对路径；失败时返回空串
 */
inline std::string get_proc_directory()
{
    std::string out;
    (void)get_proc_directory(out);
    return out;
}

/**
 * @brief 获取当前进程名称（值返回）
 * @return 进程文件名（不含扩展名）；失败时返回空串
 */
inline std::string get_proc_filename_stem()
{
    std::string out;
    (void)get_proc_filename_stem(out);
    return out;
}

}  // namespace origin::process
#endif  // ORIGIN_PROCESS_PROCESS_H
