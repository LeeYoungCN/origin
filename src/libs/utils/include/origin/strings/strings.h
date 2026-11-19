#ifndef ORIGIN_STRINGS_STRINGS_H
#define ORIGIN_STRINGS_STRINGS_H

#include <cstdarg>
#include <string>
#include <type_traits>

#include "origin/common/types/error_code_types.h"
#include "origin/common/types/type_traits.h"

namespace origin::strings {
ErrorCode va_list_to_string(const char* format, va_list args, std::string& out);
bool string_is_null_or_empty(const char* str);

template <typename T, std::enable_if_t<origin::type_traits::is_convertible_to_string_v<T>, int> = 0>
std::string type_to_string(const T& type)
{
    std::string msg;
    if constexpr (origin::type_traits::is_direct_string_type_v<T>) {
        msg = std::string(type);
    } else if constexpr (origin::type_traits::is_numeric_type_v<T>) {
        msg = std::to_string(type);
    }
    return msg;
}

}  // namespace origin::strings

#endif  // ORIGIN_STRINGS_STRINGS_H
