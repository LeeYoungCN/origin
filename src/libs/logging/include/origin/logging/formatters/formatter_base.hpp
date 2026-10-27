#ifndef ORIGIN_LOGGING_FORMATTERS_FORMATTER_BASE_HPP
#define ORIGIN_LOGGING_FORMATTERS_FORMATTER_BASE_HPP

#include <memory>

#include "origin/common/macros/compiler.h"
#include "origin/logging/detail/logging_api.h"
#include "origin/logging/formatters/formatter.hpp"
#include "origin/logging/log_msg.hpp"

#if COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace origin::logging {
class LOGGING_API FormatterBase : public Formatter {
public:
    FormatterBase() = delete;
    ~FormatterBase() override;
    explicit FormatterBase(std::unique_ptr<Formatter> pimpl);

    void format(const LogMsg& logMsg, std::string& content) override;

    [[nodiscard]] std::unique_ptr<Formatter> clone() const override;

protected:
    void throw_if_pimpl_null() const;

private:
    std::unique_ptr<Formatter> _pImpl;
};
}  // namespace origin::logging

#if COMPILER_MSVC
#pragma warning(pop)
#endif
#endif  // ORIGIN_LOGGING_FORMATTERS_FORMATTER_BASE_HPP
