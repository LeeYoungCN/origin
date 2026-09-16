#include "c/internal/common_c.hpp"
#include "logging/c/logging_c.h"
#include "logging/formatters/pattern_formatter.hpp"

using namespace origin::logging;

extern "C" {
FormatterSt *origin_create_pattern_formatter(const char *pattern)
{
    if (pattern == nullptr) {
        return new FormatterSt(new PatternFormatter());
    }
    return new FormatterSt(new PatternFormatter(pattern));
}

void origin_destroy_formatter(FormatterSt *formatter)
{
    if (formatter != nullptr) {
        if (formatter->ptr != nullptr) {
            formatter->ptr.reset();
        }
        delete formatter;
    }
}
}
