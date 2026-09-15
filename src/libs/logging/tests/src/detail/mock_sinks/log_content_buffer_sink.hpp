#ifndef LOGGING_TEST_LOG_CONTENT_BUFFER_SINK_HPP
#define LOGGING_TEST_LOG_CONTENT_BUFFER_SINK_HPP

#include <cstdint>
#include <vector>

#include "detail/mock_sinks/base_test_sink.hpp"
#include "logging/log_msg.hpp"

using namespace origin::logging;

namespace logging_test {
class LogContentBufferSink : public BaseTestSink {
public:
    LogContentBufferSink();
    ~LogContentBufferSink() override = default;
    explicit LogContentBufferSink(uint32_t capacity);

    void enable_debug_info(bool enable);
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
    bool _enanleDebugInfo{false};
};
}  // namespace logging_test

#endif  // LOGGING_TEST_LOG_CONTENT_BUFFER_SINK_HPP
