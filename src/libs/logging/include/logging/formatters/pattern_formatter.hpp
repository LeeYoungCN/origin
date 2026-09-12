#ifndef ORIGIN_LOGGING_FORMATTERS_PATTERN_FORMATTER_HPP
#define ORIGIN_LOGGING_FORMATTERS_PATTERN_FORMATTER_HPP

#include "logging/formatters/formatter_base.hpp"

namespace origin::logging {
/**
 * @brief Format log message by pattern.
 *
 * d: Date time;
 * n: Logger name;
 * l: Log level short name;
 * L: Log level full name;
 * s: File name;
 * g: File path;
 * #: Line number;
 * !: Function name;
 * t: Thread ID;
 * P: Process ID;
 * v: Log message;
 */
class LOGGING_API PatternFormatter : public FormatterBase {
public:
    static constexpr const char* DEFAULT_PATTERN =
        "[%d][%l][%s:%#]: %v";  // 默认日志格式[时间戳][等级缩写][文件名:行号]: 日志内容

public:
    PatternFormatter();
    ~PatternFormatter() override = default;

    explicit PatternFormatter(std::string_view pattern);
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_FORMATTERS_PATTERN_FORMATTER_HPP
