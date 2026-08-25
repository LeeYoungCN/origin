#ifndef ORIGIN_LOGGING_SINKS_ROTATING_FILE_SINK_H
#define ORIGIN_LOGGING_SINKS_ROTATING_FILE_SINK_H

#include <cstdint>
#include <string_view>
#include <vector>

#include "logging/sinks/sink_base.h"

namespace origin::logging {
class RotatingFileSink : public SinkBase {
public:
    static constexpr uint32_t DEFAULT_MAX_FILE_SIZE = 10 * 1024 * 1024;  // 10MB;
    static constexpr uint32_t DEFAULT_MAX_FILES = 100;                   // 最大保留100个日志文件;

    static constexpr uint32_t MAX_FILES = 20000;  // 最大保留20000个日志文件;

    static constexpr uint32_t MIN_INDEX = 1;
    static constexpr uint32_t MAX_INDEX = MAX_FILES;  // 最大索引号为20000;

public:
    RotatingFileSink();
    ~RotatingFileSink() override = default;
    explicit RotatingFileSink(std::string_view file, bool rotateOnOpen = false);
    RotatingFileSink(std::string_view file, uint32_t maxFileSize, uint32_t maxFiles,
                     bool rotateOnOpen = false);

public:
    [[nodiscard]] std::string file() const;
    [[nodiscard]] std::vector<std::string> get_file_list() const;

    void set_max_file_size(uint32_t maxFileSize) const;
    [[nodiscard]] uint32_t max_file_size() const;

    void set_max_files(uint32_t maxFiles) const;
    [[nodiscard]] uint32_t max_files() const;
};
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_SINKS_ROTATING_FILE_SINK_H
