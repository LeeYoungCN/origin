#pragma once
#ifndef ORIGIN_LOGGING_DETAILS_LOG_LEVEL_H
#define ORIGIN_LOGGING_DETAILS_LOG_LEVEL_H

namespace origin::logging {
/**
 * @enum LogLevel
 * @brief 日志级别枚举
 */
enum LogLevel {
    TRACE = 0,  ///< 跟踪信息（最详细的日志级别，适用于非常细粒度的调试信息）
    DEBUG = 1,  ///< 调试信息（仅开发/测试环境使用，包含详细调试数据）
    INFO = 2,   ///< 一般信息（正常运行时的流程性信息，如服务启动成功）
    WARN = 3,   ///< 警告信息（不影响主流程但需关注的异常，如配置项缺失使用默认值）
    ERR = 4,    ///< 错误信息（功能异常但不导致程序退出，如单次接口调用失败）
    FATAL = 5,  ///< 致命错误（导致程序无法继续运行的严重异常，如核心资源初始化失败）
    OFF = 6     ///< 关闭日志功能
};

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_DETAILS_LOG_LEVEL_H
