#include "utils/string_utils.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>

namespace origin::string {

bool string_is_null_or_empty(const char *str)
{
    return (str == nullptr || strlen(str) == 0);
}

std::string va_list_to_string(const char *format, va_list args)
{
    va_list argsCopy;

    va_copy(argsCopy, args);
    int const len = vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);

    if (len < 0) {
        throw std::runtime_error("vsnprintf failed.");
    }

    std::string message(static_cast<uint32_t>(len), '\0');
    vsnprintf(message.data(), static_cast<uint32_t>(len) + 1, format, args);
    return message;
}
}  // namespace origin::string
