#ifndef LOGGING_TEST_TID_MSG_SINK_HPP
#define LOGGING_TEST_TID_MSG_SINK_HPP

#include <map>

#include "base_test_sink.hpp"
#include "common/types/thread_types.h"
#include "logging/log_msg.hpp"

namespace logging_test {
class TidMsgMapSink : public BaseTestSink {
public:
    TidMsgMapSink() = default;
    ~TidMsgMapSink() override = default;

protected:
    void log_it(const LogMsg& logMsg) override;
    void flush_it() override {}

    const std::map<ThreadId, LogMsg>& buffer();

private:
    std::map<ThreadId, LogMsg> _buffer;
};
}  // namespace logging_test
#endif  // LOGGING_TEST_TID_MSG_SINK_HPP
