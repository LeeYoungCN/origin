#include "loggers/internal/sync_logger_impl.hpp"

#include <memory>
#include <string_view>

#include "loggers/internal/logger_impl_base.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {

SyncLoggerImpl::SyncLoggerImpl(const std::string_view name, const std::shared_ptr<Sink>& sink)
    : LoggerImplBase(name, sink)
{
}

void SyncLoggerImpl::log_it(const LogMsg& logMsg)
{
    backend_log(logMsg);
}

void SyncLoggerImpl::flush_it()
{
    backend_flush();
}
}  // namespace origin::logging
