#ifndef ORIGIN_UTILS_INTERNAL_LAST_ERROR_H
#define ORIGIN_UTILS_INTERNAL_LAST_ERROR_H

#include "origin/common/types/error_code_types.h"

namespace origin::utils::detail {

void set_last_error(ErrorCode code);
ErrorCode get_last_error();
const char* get_last_error_msg();

}  // namespace origin::utils::detail

// 内部实现使用：不对外暴露。
using origin::utils::detail::get_last_error;
using origin::utils::detail::get_last_error_msg;
using origin::utils::detail::set_last_error;

#endif  // ORIGIN_UTILS_INTERNAL_LAST_ERROR_H
