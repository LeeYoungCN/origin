#include "logging/formatters/pattern_formatter.hpp"

#include <memory>
#include <string_view>

#include "formatters/internal/pattern_formatter_impl.hpp"
#include "logging/formatters/formatter_base.hpp"

namespace origin::logging {

PatternFormatter::PatternFormatter() : FormatterBase(std::make_unique<PatternFormatterImpl>()) {}

PatternFormatter::PatternFormatter(std::string_view pattern)
    : FormatterBase(std::make_unique<PatternFormatterImpl>(pattern))
{
}

}  // namespace origin::logging
