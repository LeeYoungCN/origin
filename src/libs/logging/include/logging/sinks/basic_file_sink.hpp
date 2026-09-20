#ifndef ORIGIN_LOGGING_SINKS_BASIC_FILE_SINK_HPP
#define ORIGIN_LOGGING_SINKS_BASIC_FILE_SINK_HPP

#include <string>
#include <string_view>

#include "logging/logging_api.h"
#include "logging/sinks/sink_base.hpp"

namespace origin::logging {
class LOGGING_API BasicFileSink : public SinkBase {
public:
    BasicFileSink();
    ~BasicFileSink() override = default;
    explicit BasicFileSink(std::string_view file, bool overwrite = true);

    [[nodiscard]] std::string file() const;
};

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_SINKS_BASIC_FILE_SINK_HPP
