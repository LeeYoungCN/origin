#pragma once
#ifndef ORIGIN_LOGGING_SINKS_BASE_SINK_H
#define ORIGIN_LOGGING_SINKS_BASE_SINK_H

#include <memory>
#include <string_view>

#include "common/macros/compiler.h"
#include "logging/formatters/formatter.h"
#include "logging/log_level.h"
#include "logging/logging_api.h"
#include "logging/sinks/sink.h"

#if COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace origin::logging {
class LOGGING_API SinkBase : public Sink {
public:
    SinkBase() = delete;
    ~SinkBase() override;
    explicit SinkBase(std::unique_ptr<Sink> pImpl);

    void log(const LogMsg& logMsg) override;
    void flush() override;

    void set_pattern(std::string_view pattern) override;
    void set_formatter(std::unique_ptr<Formatter> formatter) override;

    [[nodiscard]] bool should_log(LogLevel level) const override;
    void set_level(LogLevel level) override;
    [[nodiscard]] LogLevel level() const override;
    [[nodiscard]] std::string_view param_str() const override;

protected:
    void throw_if_pimpl_null() const;
    std::unique_ptr<Sink> _pImpl;
};
}  // namespace origin::logging

#if COMPILER_MSVC
#pragma warning(pop)
#endif

#endif  // ORIGIN_LOGGING_SINKS_BASE_SINK_H
