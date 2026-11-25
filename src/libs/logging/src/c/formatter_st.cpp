#include "origin/logging/c/logging_c.h"
#include "origin/logging/formatters/pattern_formatter.hpp"

#include "c/internal/common_c.hpp"  // NOLINT(misc-include-cleaner): 这里定义了 C API 的桥接结构体

using namespace origin::logging;

extern "C" {
FormatterSt* origin_create_pattern_formatter(const char* pattern)
{
    try {
        if (pattern == nullptr) {
            return new FormatterSt(new PatternFormatter());
        }
        return new FormatterSt(new PatternFormatter(pattern));
    } catch (...) {
        return nullptr;
    }
}

void origin_destroy_formatter(FormatterSt* formatter)
{
    if (formatter == nullptr) {
        return;
    }
    formatter->ptr.reset();
    delete formatter;
}
}
