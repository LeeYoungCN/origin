/**
 * @file thread.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-08-07
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef ORIGIN_THREAD_THREAD_H
#define ORIGIN_THREAD_THREAD_H

#include <string>

#include "origin/common/types/error_code_types.h"
#include "origin/common/types/thread_types.h"

namespace origin::thread {

/**
 * @brief 获取当前内核态线程ID
 *
 * @return 当前线程ID
 */
ThreadId get_curr_thread_id();

/**
 * @brief 设置线程名称
 *
 * @param name 线程名称
 * @return 成功返回 ERR_COMM_SUCCESS
 */
ErrorCode set_curr_thread_name(const char* name);

/**
 * @brief 获取当前线程名称
 *
 * @param[out] out 当前线程名称，未设置则为空串
 * @return 成功返回 ERR_COMM_SUCCESS
 */
ErrorCode get_curr_thread_name(std::string& out);

/**
 * @brief 获取当前线程名称（值返回）
 * @return 当前线程名称，未设置或失败时为空串
 */
inline std::string get_curr_thread_name()
{
    std::string out;
    (void)get_curr_thread_name(out);
    return out;
}

}  // namespace origin::thread

#endif  // ORIGIN_THREAD_THREAD_H
