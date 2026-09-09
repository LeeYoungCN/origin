#include "loggers/internal/sync_logger_impl.h"

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include "loggers/internal/logger_impl_base.h"
#include "logging/log_msg.h"
#include "logging/sinks/sink.h"

namespace origin::logging {

SyncLoggerImpl::SyncLoggerImpl(std::string_view name) : LoggerImplBase(name) {}

SyncLoggerImpl::SyncLoggerImpl(std::string_view name, const std::shared_ptr<Sink>& sink)
    : LoggerImplBase(name, sink)
{
}

SyncLoggerImpl::SyncLoggerImpl(std::string_view name,
                               const std::vector<std::shared_ptr<Sink>>& sinks)
    : LoggerImplBase(name, sinks)
{
}
SyncLoggerImpl::SyncLoggerImpl(std::string_view name,
                               const std::initializer_list<std::shared_ptr<Sink>>& sinks)
    : LoggerImplBase(name, sinks)
{
}

void SyncLoggerImpl::log_it(const LogMsg& logMsg)
{
    backend_log(logMsg);

    if (should_flush(logMsg.level)) {
        backend_flush();
    }
}

void SyncLoggerImpl::flush_it()
{
    backend_flush();
}
}  // namespace origin::logging
