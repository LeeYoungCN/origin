#include "common/debug/debug_logger.h"

#include <cstdarg>
#include <mutex>
#include <string>

#include "common/base/singleton.h"
#include "common/macros/compiler.h"

#if OS_WINDOWS
#include <windows.h>
#elif OS_LINUX
#include <sys/syscall.h>
#include <unistd.h>

#include <chrono>
#elif OS_MACOS
#include <pthread.h>

#include <chrono>
#else
#error "Unsupport system"
#endif

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <format>
#include <iostream>

namespace {
int64_t get_now_timestamp_ms()
{
#if OS_WINDOWS
    FILETIME ft;
    // 获取当前系统时间，以FILETIME格式存储（从Windows纪元1601-01-01 00:00:00开始的100纳秒间隔数）
    GetSystemTimeAsFileTime(&ft);

    // 将FILETIME的高低位 DWORD 合并为64位无符号整数，得到完整的100纳秒单位时间戳
    constexpr int FILETIME_HIGH_SHIFT_BITS = 32;  // FILETIME高32位左移位数
    uint64_t file_time =
        (static_cast<uint64_t>(ft.dwHighDateTime) << FILETIME_HIGH_SHIFT_BITS) | ft.dwLowDateTime;

    // 转换为Unix时间戳（毫秒级）：
    // 1. 先将100纳秒单位转换为毫秒（除以10000，因1毫秒=10000×100纳秒）
    // 2. 减去Windows纪元到Unix纪元（1970-01-01 00:00:00）的毫秒差值，得到标准Unix时间戳
    constexpr uint64_t HUNDRED_NANOSECONDS_PER_MILLISECOND = 10000;
    constexpr uint64_t WINDOWS_EPOCH_TO_UNIX_EPOCH_MS = 11644473600000ULL;
    return static_cast<int64_t>((file_time / HUNDRED_NANOSECONDS_PER_MILLISECOND)  // 转换为毫秒
                                - WINDOWS_EPOCH_TO_UNIX_EPOCH_MS                   // 校正到Unix纪元
    );
#else
    std::chrono::time_point now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<int64_t>(ms.count());
#endif
}

std::string time_string()
{
    constexpr int64_t MILLIS_PER_SECOND = 1000;

    auto timestamp = get_now_timestamp_ms();
    auto timer = static_cast<std::time_t>(timestamp / MILLIS_PER_SECOND);
    auto millis = static_cast<int32_t>(timestamp % MILLIS_PER_SECOND);

    std::tm ltm{};
#if OS_WINDOWS
    // Windows 使用 localtime_s
    localtime_s(&ltm, &timer);
#else
    // Linux/macOS 使用 localtime_r
    localtime_r(&timer, &ltm);
#endif
    std::stringstream timeSs;

    timeSs << std::put_time(&ltm, "%Y-%m-%d %H:%M:%S");
    timeSs << "." << std::setfill('0') << std::setw(3) << millis;

    return timeSs.str();
}

size_t get_current_tid()
{
#if OS_WINDOWS
    return static_cast<size_t>(GetCurrentThreadId());
#elif OS_LINUX
    return static_cast<size_t>(syscall(SYS_gettid));
#elif OS_MACOS
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<size_t>(tid);
#else
    return 0;
#endif
}

std::string va_list_to_string(const char* format, va_list args)
{
    va_list argsCopy;

    va_copy(argsCopy, args);
    int len = vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);

    if (len < 0) {
        throw std::runtime_error("vsnprintf failed.");
    }

    std::string message(static_cast<uint32_t>(len), '\0');
    vsnprintf(message.data(), static_cast<uint32_t>(len) + 1, format, args);
    return message;
}

const char* get_debug_log_lvl_str(OriginDbgLvl level)
{
    switch (level) {
        case ORG_DBG_LVL_TRACE:
            return "T";
        case ORG_DBG_LVL_DEBUG:
            return "D";
        case ORG_DBG_LVL_INFO:
            return "I";
        case ORG_DBG_LVL_WARN:
            return "W";
        case ORG_DBG_LVL_ERR:
            return "E";
        case ORG_DBG_LVL_FATAL:
            return "F";
        case ORG_DBG_LVL_OFF:
            return "O";
        default:
            return "UNKNOWN";
    }
}

class DebugLoggerImpl : public origin::common::base::SingletonBase<DebugLoggerImpl> {
    friend class origin::common::base::SingletonBase<DebugLoggerImpl>;

public:
    void set_debug_log_level(OriginDbgLvl level) { _logLevel = level; }

    bool should_log(OriginDbgLvl level)
    {
        return (_logLevel != OriginDbgLvl::ORG_DBG_LVL_OFF && level >= _logLevel);
    }

    void log_va(const char* file, int line, const char* func, OriginDbgLvl level,
                const char* format, va_list args)
    {
        log(file, line, func, level, va_list_to_string(format, args));
    }

    void log(const char* file, int line, const char* func, OriginDbgLvl level,
             const std::string& message)
    {
        auto logmsg = format_log(level, message, file, line, func);
        std::lock_guard lock(_mtx);
        std::cout << logmsg << std::endl;
    }

private:
    DebugLoggerImpl() = default;
    ~DebugLoggerImpl() override = default;

private:
    std::string format_log(OriginDbgLvl level, const std::string& message,
                           const char* file = nullptr, int line = 0, const char* func = nullptr)
    {
        if (file != nullptr) {
            return std::format("[{}][{}][Tid: {:#x}][{}:{}][{}]: {}",
                               time_string(),
                               get_debug_log_lvl_str(level),
                               get_current_tid(),
                               std::filesystem::path(file).filename().string(),
                               line,
                               func,
                               message);
        } else {
            return std::format("[{}] [{}] [Tid: {:#x}]: {}",
                               time_string(),
                               get_debug_log_lvl_str(level),
                               get_current_tid(),
                               message);
        }
    }

private:
    OriginDbgLvl _logLevel = OriginDbgLvl::ORG_DBG_LVL_INFO;
    std::mutex _mtx;
};

}  // namespace

extern "C" {
void origin_set_debug_logger_level(OriginDbgLvl level)
{
    DebugLoggerImpl::instance().set_debug_log_level(level);
}

bool origin_debug_logger_should_log(OriginDbgLvl level)
{
    return DebugLoggerImpl::instance().should_log(level);
}

void origin_debug_logger_c(const char* file, int line, const char* func, OriginDbgLvl level,
                           const char* format, ...)
{
    if (!DebugLoggerImpl::instance().should_log(level)) {
        return;
    }

    va_list args;
    va_start(args, format);
    DebugLoggerImpl::instance().log_va(file, line, func, level, format, args);
    va_end(args);
}
}

void origin_debug_logger_log(const char* file, int line, const char* func, OriginDbgLvl level,
                             const std::string& message)
{
    DebugLoggerImpl::instance().log(file, line, func, level, message);
}
