#ifndef TEST_LOGGING_TEST_UTILS_LOG_CONTENT_BUFFER_SINK_H
#define TEST_LOGGING_TEST_UTILS_LOG_CONTENT_BUFFER_SINK_H

#include <sys/types.h>

#include <cstdint>
#include <vector>

#include "base_test_sink.h"
#include "internal/log_msg.h"

using namespace origin::logging;

namespace logging_test {
class LogContentBufferSink : public BaseTestSink {
public:
    LogContentBufferSink();
    ~LogContentBufferSink() override = default;
    explicit LogContentBufferSink(uint32_t capacity);

    [[nodiscard]] uint32_t capacity() const;
    const std::vector<std::string>& buffer();
    const std::vector<std::string>& disk();
    void clear();

protected:
    void log_it(const LogMsg& logMsg) override;
    void flush_it() override;

    void sink_it(std::string_view message);

private:
    uint32_t _capacity;
    std::vector<std::string> _buffer;
    std::vector<std::string> _disk;
};
}  // namespace logging_test

#endif  // TEST_LOGGING_TEST_UTILS_LOG_CONTENT_BUFFER_SINK_H
