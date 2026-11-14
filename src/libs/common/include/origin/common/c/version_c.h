#ifndef ORIGIN_COMMON_C_VERSION_C_H
#define ORIGIN_COMMON_C_VERSION_C_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stddef.h>  // size_t

#include "origin/common/common_api.h"

/**
 * @brief 库版本字符串，形如 "1.0.1"，开发版形如 "1.0.1-dev+g7b4ef62"。
 *
 * 返回静态存储的字符串，进程生命周期内有效，调用方不需要释放。
 */
COMMON_API const char *origin_version();

/**
 * @brief 版本信息汇总（含构建时间），形如
 *        "Origin 1.0.1 (build 2026-11-14T12:11:59+0800)"。
 *
 * @param buf 输出缓冲区；传 NULL 可用于查询所需长度（此时 len 可为 0）。
 * @param len 缓冲区大小（含结尾 NUL）。
 * @return 写入的字符数（不含结尾 NUL）；缓冲区不足时返回实际写入长度并截断。
 */
COMMON_API size_t origin_version_info(char *buf, size_t len);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // ORIGIN_COMMON_C_VERSION_C_H
