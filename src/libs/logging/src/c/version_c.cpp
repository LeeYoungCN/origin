/**
 * @file version_c.cpp
 * @brief Logging 组件版本信息的 C 接口实现
 *
 */
#include "origin/logging/c/logging_c.h"

#include "origin/logging/version.h"

extern "C" {

const char *origin_logging_version()
{
    return ORIGIN_VERSION_STRING;
}

const char *origin_logging_build_time()
{
    return ORIGIN_BUILD_TIMESTAMP;
}

}  // extern "C"