#pragma once
#ifndef ORIGIN_LOGGING_SINKS_DAILY_FILE_SINK_HPP
#define ORIGIN_LOGGING_SINKS_DAILY_FILE_SINK_HPP
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "logging/logging_api.h"
#include "logging/sinks/sink_base.hpp"

namespace origin::logging {
/**
 * @brief: 按照日期滚动日志。
 */
class LOGGING_API DailyFileSink : public SinkBase {
public:
    static constexpr uint32_t DEFAULT_ROTATION_HOUR = 0;    // 默认滚动时间每天00:00
    static constexpr uint32_t DEFAULT_ROTATION_MINUTE = 0;  // 默认滚动时间每天00:00
    static constexpr uint32_t DEFAULT_MAX_FILES = 30;       // 默认最大保留30天日志文件
    static constexpr uint32_t MAX_FILES = 20000;            // 最大保留20000个日志文件

public:
    DailyFileSink();
    explicit DailyFileSink(std::string_view file, bool overwrite = false);
    DailyFileSink(std::string_view file, uint32_t hour, uint32_t minute, bool overwrite = false);
    DailyFileSink(std::string_view file, uint32_t hour, uint32_t minute, uint32_t maxFiles,
                  bool overwrite = false);
    ~DailyFileSink() override = default;

    [[nodiscard]] std::string file() const;
    [[nodiscard]] std::vector<std::string> get_file_list() const;

    void set_max_files(uint32_t maxFiles);
    [[nodiscard]] uint32_t max_files() const;
};

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_SINKS_DAILY_FILE_SINK_HPP
