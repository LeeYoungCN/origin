/**
 * @file version_c.cpp
 * @brief 版本信息查询的 C 接口实现
 */
#include "origin/common/c/version_c.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "origin/common/version.h"

extern "C" {

const char *origin_version()
{
    return origin::version().data();
}

size_t origin_version_info(char *buf, const size_t len)
{
    const std::string info = origin::version_info();
    if (buf == nullptr || len == 0) {
        return info.size();
    }

    const size_t written = std::min(info.size(), len - 1);
    std::memcpy(buf, info.data(), written);
    buf[written] = '\0';
    return written;
}

}  // extern "C"
