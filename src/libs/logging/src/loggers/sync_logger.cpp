#include "logging/loggers/sync_logger.hpp"

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include "loggers/internal/sync_logger_impl.hpp"
#include "logging/loggers/logger_base.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {
SyncLogger::SyncLogger(const std::string_view name, const std::shared_ptr<Sink>& sink)
    : LoggerBase(std::make_shared<SyncLoggerImpl>(name, sink))
{
}

SyncLogger::SyncLogger(const std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks)
    : LoggerBase(std::make_shared<SyncLoggerImpl>(name, sinks.begin(), sinks.end()))
{
}

SyncLogger::SyncLogger(const std::string_view name,
                       const std::initializer_list<std::shared_ptr<Sink>>& sinks)
    : LoggerBase(std::make_shared<SyncLoggerImpl>(name, sinks.begin(), sinks.end()))
{
}

}  // namespace origin::logging
