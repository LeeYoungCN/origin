/**
 * @file thread_utils.h
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
 */
ErrorCode set_curr_thread_name(const char* name);

/**
 * @brief 获取当前线程名称
 *
 * @return 当前线程名称，未设置则为空
 */
ErrorCode get_curr_thread_name(std::string& out);

#ifdef UTILS_BUILD_SHARED
// 内部诊断状态：仅库内部实现使用，不对外暴露。
void set_thread_last_err(ErrorCode errcode);
ErrorCode get_thread_last_err();
const char *get_thread_last_err_msg();
#endif

}  // namespace origin::thread

#endif  // ORIGIN_THREAD_THREAD_H
