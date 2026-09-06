#ifndef ORIGIN_LOGGING_LOGGERS_SYNC_LOGGER_H
#define ORIGIN_LOGGING_LOGGERS_SYNC_LOGGER_H

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include "logging/loggers/logger_base.h"
#include "logging/sinks/sink.h"

namespace origin::logging {
class LOGGING_API SyncLogger : public LoggerBase {
public:
    SyncLogger() = delete;
    ~SyncLogger() override = default;

    explicit SyncLogger(std::string_view name);

    SyncLogger(std::string_view name, const std::shared_ptr<Sink>& sink);

    SyncLogger(std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks);

    SyncLogger(std::string_view name, const std::initializer_list<std::shared_ptr<Sink>>& sinks);
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGERS_SYNC_LOGGER_H
