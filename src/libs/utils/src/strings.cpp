#include "origin/strings/strings.h"
#include "origin/common/common_error_code.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>

namespace origin::strings {

bool string_is_null_or_empty(const char *str)
{
    return (str == nullptr || strlen(str) == 0);
}

ErrorCode va_list_to_string(const char *format, va_list args, std::string& out)
{
    va_list argsCopy;

    va_copy(argsCopy, args);
    int const len = vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);

    if (len < 0) {
        return ERR_COMM_SYSTEM_ERROR;
    }

    std::string message(static_cast<uint32_t>(len), '\0');
    vsnprintf(message.data(), static_cast<uint32_t>(len) + 1, format, args);
    out = std::move(message);
    return ERR_COMM_SUCCESS;
}
}  // namespace origin::strings
