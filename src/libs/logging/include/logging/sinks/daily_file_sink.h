#pragma once
#ifndef ORIGIN_LOGGING_SINKS_DAILY_FILE_SINK_H
#define ORIGIN_LOGGING_SINKS_DAILY_FILE_SINK_H
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "logging/sinks/sink_base.h"

namespace origin::logging {
/**
 * @brief: 按照日期滚动日志。
 */
class DailyFileSink : public SinkBase {
public:
    static constexpr uint32_t DEFAULT_ROTATION_HOUR = 0;
    static constexpr uint32_t DEFAULT_ROTATION_MINUTE = 0;
    static constexpr uint32_t DEFAULT_MAX_FILES = 30;
    static constexpr uint32_t MAX_FILES = 20000;

public:
    DailyFileSink();
    explicit DailyFileSink(std::string_view file, bool overwrite = false);
    DailyFileSink(std::string_view file, uint32_t hour, uint32_t minute, bool overwrite = false);
    DailyFileSink(std::string_view file, uint32_t hour, uint32_t minute, uint32_t maxFiles,
                  bool overwrite = false);
    ~DailyFileSink() override = default;

    [[nodiscard]] std::string file() const;
    [[nodiscard]] std::vector<std::string> get_file_list() const;

    void set_max_files(uint32_t maxFiles) const;
    [[nodiscard]] uint32_t max_files() const;
};

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_SINKS_DAILY_FILE_SINK_H
