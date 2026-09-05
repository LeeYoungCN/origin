#ifndef COMMON_CONTAINER_CSTRING_HPP
#define COMMON_CONTAINER_CSTRING_HPP
#include <atomic>
#include <cstdarg>
#include <cstddef>
#include <format>

#include "common/common_error_code.h"
#include "common/debug/debug_logger.h"
#include "common/types/error_code_types.h"

template <size_t size>
class CString {
public:
    CString() = default;
    ~CString() = default;

    CString(const char *format, ...)
    {
        if (format != nullptr) {
            va_list ap;
            va_start(ap, format);
            m_lastErrcode = append_va(format, ap);
            va_end(ap);
        }
        if (m_lastErrcode != ERR_COMM_SUCCESS) {
            ORIGIN_DEBUG_ERR("Construct cstring failed. errcode: {:#x}, msg: {}.",
                             m_lastErrcode,
                             get_comm_err_msg(m_lastErrcode));
        }
    }

    ErrorCode append(const char *format, ...)
    {
        if (format != nullptr) {
            va_list ap;
            va_start(ap, format);
            append_va(format, ap);
            va_end(ap);
        }
        return m_lastErrcode;
    }

    template <typename... Args>
    void append(std::format_string<Args...> fmt, Args &&...args)
    {
        auto rst = std::format_to_n(
            m_cstr + m_length, m_capacity - m_length - 1, fmt, std::forward(args)...);
        *rst.out = '\0';
        m_length += rst.size;
    }

    size_t length() { return m_length; }

    size_t capacity() { return m_capacity; }

    void reset()
    {
        m_cstr[0] = '\0';
        m_length = 0;
    }

    ErrorCode last_error() { return m_lastErrcode; }

private:
    ErrorCode append_va(const char *format, va_list ap)
    {
        size_t fmtLen = 0;
        if (format != nullptr) {
            fmtLen = static_cast<size_t>(
                vsnprintf(m_cstr + m_length, m_capacity - m_length, format, ap));
        }

        if (m_length + fmtLen < m_capacity) {
            m_length += fmtLen;
            m_lastErrcode = ERR_COMM_SUCCESS;
        } else {
            m_lastErrcode = ERR_COMM_CTN_OVERFLOW;
            ORIGIN_DEBUG_ERR("CString overflow. cap: {}, len: {}, errcode: {:#x}.",
                             m_capacity,
                             m_length + fmtLen,
                             m_lastErrcode);
        }
        m_cstr[m_length] = '\0';
        return m_lastErrcode;
    }

    ErrorCode m_lastErrcode{ERR_COMM_SUCCESS};
    char m_cstr[size]{};
    std::atomic<size_t> m_length{0};
    const size_t m_capacity{size};
};
#endif  // COMMON_CONTAINER_CSTRING_HPP
