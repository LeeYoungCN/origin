#ifndef ORIGIN_LOGGING_FORMATTERS_FORMATTER_HPP
#define ORIGIN_LOGGING_FORMATTERS_FORMATTER_HPP

#include <memory>
#include <string>

#include "common/macros/compiler.h"
#include "logging/detail/logging_api.h"

#if COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace origin::logging {
struct LogMsg;
class LOGGING_API Formatter {
public:
    Formatter() = default;
    virtual ~Formatter() = default;

    virtual void format(const LogMsg& logMsg, std::string& content) = 0;

    [[nodiscard]] virtual std::unique_ptr<Formatter> clone() const = 0;
};
}  // namespace origin::logging

#if COMPILER_MSVC
#pragma warning(pop)
#endif

#endif  // ORIGIN_LOGGING_FORMATTERS_FORMATTER_HPP
