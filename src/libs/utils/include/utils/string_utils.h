#ifndef ORIGIN_UTILS_STRING_UTILS_H
#define ORIGIN_UTILS_STRING_UTILS_H

#include <cstdarg>
#include <string>
#include <type_traits>

#include "common/types/type_traits.h"

namespace origin::string {
std::string va_list_to_string(const char* format, va_list args);
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

}  // namespace origin::string

#endif  // ORIGIN_UTILS_STRING_UTILS_H
