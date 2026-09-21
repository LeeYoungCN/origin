#ifndef ORIGIN_LOGGING_LOGGERS_SYNC_LOGGER_HPP
#define ORIGIN_LOGGING_LOGGERS_SYNC_LOGGER_HPP

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include "logging/detail/logging_api.h"
#include "logging/loggers/logger_base.hpp"
#include "logging/sinks/sink.hpp"

namespace origin::logging {
class LOGGING_API SyncLogger : public LoggerBase {
public:
    SyncLogger() = delete;
    ~SyncLogger() override = default;

    SyncLogger(std::string_view name, const std::shared_ptr<Sink>& sink);

    SyncLogger(std::string_view name, const std::vector<std::shared_ptr<Sink>>& sinks);

    SyncLogger(std::string_view name, const std::initializer_list<std::shared_ptr<Sink>>& sinks);
};
}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_LOGGERS_SYNC_LOGGER_HPP
