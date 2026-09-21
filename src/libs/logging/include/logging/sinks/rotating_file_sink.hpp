#ifndef ORIGIN_LOGGING_SINKS_ROTATING_FILE_SINK_HPP
#define ORIGIN_LOGGING_SINKS_ROTATING_FILE_SINK_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "logging/detail/constants.h"
#include "logging/detail/logging_api.h"
#include "logging/sinks/sink_base.hpp"

namespace origin::logging {
class LOGGING_API RotatingFileSink : public SinkBase {
public:
    static constexpr uint32_t DEFAULT_MAX_FILE_SIZE =
        ROTATING_FILE_SINK_DEFAULT_MAX_FILE_SIZE;  // 默认日志文件大小10MB
    static constexpr uint32_t DEFAULT_MAX_FILES =
        ROTATING_FILE_SINK_DEFAULT_MAX_FILES;  // 默认最大保留100个日志文件
    static constexpr uint32_t MAX_FILES = ROTATING_FILE_SINK_MAX_FILES;  // 最大保留20000个日志文件
    static constexpr uint32_t MIN_INDEX =
        ROTATING_FILE_SINK_MIN_INDEX;  // 日志文件索引号 [1, 20000]
    static constexpr uint32_t MAX_INDEX =
        ROTATING_FILE_SINK_MAX_INDEX;  // 日志文件索引号 [1, 20000]

    RotatingFileSink();
    ~RotatingFileSink() override = default;
    explicit RotatingFileSink(std::string_view file, bool rotateOnOpen = false);
    RotatingFileSink(std::string_view file, uint32_t maxFileSize, uint32_t maxFiles,
                     bool rotateOnOpen = false);

    [[nodiscard]] std::string file() const;
    [[nodiscard]] std::vector<std::string> get_file_list() const;

    void set_max_file_size(uint32_t maxFileSize) const;
    [[nodiscard]] uint32_t max_file_size() const;

    void set_max_files(uint32_t maxFiles) const;
    [[nodiscard]] uint32_t max_files() const;
};
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_SINKS_ROTATING_FILE_SINK_HPP
