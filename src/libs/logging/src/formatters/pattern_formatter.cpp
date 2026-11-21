#include "origin/logging/formatters/pattern_formatter.hpp"

#include <memory>
#include <string_view>

#include "origin/logging/formatters/formatter_base.hpp"

#include "formatters/internal/pattern_formatter_impl.hpp"

namespace origin::logging {

PatternFormatter::PatternFormatter() : FormatterBase(std::make_unique<PatternFormatterImpl>()) {}

PatternFormatter::PatternFormatter(const std::string_view pattern)
    : FormatterBase(std::make_unique<PatternFormatterImpl>(pattern))
{
}

}  // namespace origin::logging
