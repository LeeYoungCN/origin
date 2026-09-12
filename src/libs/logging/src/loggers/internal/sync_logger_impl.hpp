#ifndef ORIGIN_LOGGING_LOGGERS_INTERNAL_SYNC_LOGGER_IMPL_HPP
#define ORIGIN_LOGGING_LOGGERS_INTERNAL_SYNC_LOGGER_IMPL_HPP

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include "loggers/internal/logger_impl_base.hpp"
#include "logging/log_msg.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {
class SyncLoggerImpl : public LoggerImplBase {
public:
    SyncLoggerImpl() = delete;
    ~SyncLoggerImpl() override = default;

    explicit SyncLoggerImpl(std::string_view name);
    SyncLoggerImpl(std::string_view name, const std::shared_ptr<Sink>& sink);
    SyncLoggerImpl(std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks);
    SyncLoggerImpl(std::string_view name,
                   const std::initializer_list<std::shared_ptr<Sink>>& sinks);

protected:
    void log_it(const LogMsg& logMsg) override;
    void flush_it() override;
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGERS_INTERNAL_SYNC_LOGGER_IMPL_HPP
