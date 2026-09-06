#ifndef ORIGIN_LOGGING_FORMATTERS_PATTERN_FORMATTER_H
#define ORIGIN_LOGGING_FORMATTERS_PATTERN_FORMATTER_H

#include "logging/formatters/formatter_base.h"

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
    static constexpr const char* DEFAULT_PATTERN = "[%d][%l][%s:%#]: %v";

public:
    PatternFormatter();
    ~PatternFormatter() override = default;

    explicit PatternFormatter(std::string_view pattern);
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_FORMATTERS_PATTERN_FORMATTER_H
