#ifndef ORIGIN_LOGGING_FORMATTERS_INTERNAL_PATTERN_FORMATTER_IMPL_H
#define ORIGIN_LOGGING_FORMATTERS_INTERNAL_PATTERN_FORMATTER_IMPL_H

#include <memory>

#include "logging/formatters/formatter.h"
#include "logging/formatters/pattern_formatter.h"
#include "logging/log_msg.h"

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
class PatternFormatterImpl : public Formatter {
public:
    PatternFormatterImpl() = default;
    ~PatternFormatterImpl() override = default;

    explicit PatternFormatterImpl(std::string_view pattern);

    void format(const LogMsg& logMsg, std::string& logContent) override;
    [[nodiscard]] std::unique_ptr<Formatter> clone() const override;

private:
    static void log_msg_to_content(char symbol, const LogMsg& logMsg, std::string& logContent);
    static void format_time(const LogMsg& logMsg, std::string& logContent);

    const std::string _pattern{PatternFormatter::DEFAULT_PATTERN};
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_FORMATTERS_INTERNAL_PATTERN_FORMATTER_IMPL_H
