/**
 * @file version_c.cpp
 * @brief 版本信息查询的 C 接口实现
 */
#include "origin/common/c/version_c.h"

#include "origin/common/version.h"

extern "C" {

uint32_t origin_c_abi_version(void)
{
    return ORIGIN_C_ABI_VERSION;
}

const char *origin_version(void)
{
    return origin::version().data();
}

const char *origin_version_commit(void)
{
    const origin::BuildInfo info = origin::build_info();
    // 空字符串而不是 nullptr：绑定的字符串转换函数通常不接受 NULL
    return info.commit.empty() ? "" : info.commit.data();
}

const char *origin_build_time(void)
{
    return origin::build_info().build_time.data();
}

}  // extern "C"
