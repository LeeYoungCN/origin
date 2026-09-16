#ifndef ORIGIN_LOGGING_LOGGERS_INTERNAL_SYNC_LOGGER_IMPL_HPP
#define ORIGIN_LOGGING_LOGGERS_INTERNAL_SYNC_LOGGER_IMPL_HPP

#include <memory>
#include <string_view>

#include "loggers/internal/logger_impl_base.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {
class SyncLoggerImpl : public LoggerImplBase {
public:
    SyncLoggerImpl() = delete;
    ~SyncLoggerImpl() override = default;

    SyncLoggerImpl(std::string_view name, const std::shared_ptr<Sink>& sink);

    template <typename It>
    SyncLoggerImpl(std::string_view name, It begin, It end) : LoggerImplBase(name, begin, end)
    {
    }

protected:
    void log_it(const LogMsg& logMsg) override;
    void flush_it() override;
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGERS_INTERNAL_SYNC_LOGGER_IMPL_HPP
