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
#pragma once

#ifndef UTILS_THREAD_UTILS_H
#define UTILS_THREAD_UTILS_H

#include "common/types/error_code_types.h"
#include "common/types/thread_types.h"

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
void set_curr_thread_name(const char *name);

/**
 * @brief 获取当前线程名称
 *
 * @return 当前线程名称，未设置则为空
 */
const char *get_curr_thread_name();

void set_thread_last_err(ErrorCode errcode);
ErrorCode get_thread_last_err();
const char *get_thread_last_err_msg();

}  // namespace origin::thread

#endif  // UTILS_THREAD_UTILS_H
