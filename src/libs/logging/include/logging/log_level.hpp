#ifndef ORIGIN_LOGGING_LOG_LEVEL_HPP
#define ORIGIN_LOGGING_LOG_LEVEL_HPP

#include <cstdint>

#include "logging/detail/constants.h"
#include "logging/detail/logging_api.h"

namespace origin::logging {
/**
 * @enum LogLevel
 * @brief 日志级别枚举
 */
enum class LogLevel : uint32_t {
    TRACE = LOG_LEVEL_VALUE_TRACE,  // 跟踪信息(最详细的日志级别，适用于非常细粒度的调试信息)
    DEBUG = LOG_LEVEL_VALUE_DEBUG,  // 调试信息(仅开发/测试环境使用，包含详细调试数据)
    INFO = LOG_LEVEL_VALUE_INFO,    // 一般信息(正常运行时的流程性信息，如服务启动成功)
    WARN = LOG_LEVEL_VALUE_WARN,    // 警告信息(不影响主流程但需关注的异常，如配置项缺失使用默认值)
    ERR = LOG_LEVEL_VALUE_ERROR,    // 错误信息(功能异常但不导致程序退出，如单次接口调用失败)
    FATAL = LOG_LEVEL_VALUE_FATAL,  // 致命错误(导致程序无法继续运行的严重异常)
    OFF = LOG_LEVEL_VALUE_OFF       // 关闭日志功能
};

LOGGING_API const char* log_level_full_name(LogLevel level);
LOGGING_API const char* log_level_abbr_name(LogLevel level);
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOG_LEVEL_HPP
