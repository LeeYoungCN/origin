#include "origin/strings/strings.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

#include "origin/common/common_error_code.h"
#include "origin/common/types/error_code_types.h"

namespace origin::strings {

bool string_is_null_or_empty(const char* str)
{
    return (str == nullptr || strlen(str) == 0);
}

ErrorCode va_list_to_string(const char* format, va_list args, std::string& out)
{
    if (format == nullptr) {
        return ERR_COMM_PARAM_NULL;
    }

    va_list argsCopy;
    va_copy(argsCopy, args);
    int const len = vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);

    if (len < 0) {
        return ERR_COMM_SYSTEM_ERROR;
    }

    // 为结尾 NUL 多留 1 字节；第二次格式化后按实际写入长度收缩。
    std::string message(static_cast<std::size_t>(len) + 1, '\0');
    int const written = vsnprintf(message.data(), message.size(), format, args);
    if (written < 0 || written > len) {
        return ERR_COMM_SYSTEM_ERROR;
    }
    message.resize(static_cast<std::size_t>(written));
    out = std::move(message);
    return ERR_COMM_SUCCESS;
}
}  // namespace origin::strings
