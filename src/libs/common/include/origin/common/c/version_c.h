#ifndef ORIGIN_COMMON_C_VERSION_C_H
#define ORIGIN_COMMON_C_VERSION_C_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>  // uint32_t

#include "origin/common/common_api.h"

/**
 * @brief C ABI 的版本号，与语义化版本（ORIGIN_VERSION_STRING）无关。
 *
 * 只要 C 头文件里的函数签名、结构体布局或枚举取值发生不兼容变化就 +1。
 * 语言绑定应在加载库后先调用 origin_c_abi_version()，与绑定编译时的
 * ORIGIN_C_ABI_VERSION 比对，不匹配就明确报错，而不是等到运行时崩溃。
 */
#define ORIGIN_C_ABI_VERSION 1u

/**
 * @brief 返回运行期库的 C ABI 版本。
 */
COMMON_API uint32_t origin_c_abi_version(void);

/**
 * @brief 完整版本：发布版 "1.0.1"，开发版 "1.0.1-dev+g7b4ef62"。
 *
 * 开发版工作区有未提交修改时，串尾会带 "-dirty"。
 * 返回库内静态存储，进程生命周期内有效，调用方不需要释放。
 */
COMMON_API const char *origin_version(void);

/**
 * @brief 短 git 提交号；发布构建或没有 git 时返回空字符串。
 *
 * 返回库内静态存储，调用方不需要释放。
 */
COMMON_API const char *origin_version_commit(void);

/**
 * @brief 构建时间（configure 阶段），形如 "2026-11-14T15:53:08+0800"。
 *
 * 返回库内静态存储，调用方不需要释放。
 */
COMMON_API const char *origin_build_time(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // ORIGIN_COMMON_C_VERSION_C_H
