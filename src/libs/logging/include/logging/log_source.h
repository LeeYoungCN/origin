#ifndef ORIGIN_LOGGING_LOG_SOURCE_H
#define ORIGIN_LOGGING_LOG_SOURCE_H

#include <string>

namespace origin::logging {
struct LogSource {
    std::string file;
    int line{0};
    std::string func;
};
}  // namespace origin::logging

#define LOG_SRC_LOCAL                    \
    origin::logging::LogSource           \
    {                                    \
        __FILE__, __LINE__, __FUNCTION__ \
    }
#define LOG_SRC_EMPTY origin::logging::LogSource()

#endif  // ORIGIN_LOGGING_LOG_SOURCE_H
