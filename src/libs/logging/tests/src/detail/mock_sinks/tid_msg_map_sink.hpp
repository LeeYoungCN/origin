#ifndef LOGGING_TEST_TID_MSG_SINK_HPP
#define LOGGING_TEST_TID_MSG_SINK_HPP

#include <map>
#include <vector>

#include "detail/mock_sinks/base_test_sink.hpp"
#include "origin/common/types/thread_types.h"
#include "origin/logging/log_msg.hpp"

namespace logging_test {
class TidMsgMapSink : public BaseTestSink {
public:
    TidMsgMapSink() : BaseTestSink("TidMsgMapSink.") {};
    ~TidMsgMapSink() override = default;

    const std::map<ThreadId, std::vector<LogMsg>>& buffer();

protected:
    void clear_it() override;
    void log_it(const LogMsg& logMsg) override;
    void flush_it() override {}

private:
    std::map<ThreadId, std::vector<LogMsg>> _buffer;
};
}  // namespace logging_test
#endif  // LOGGING_TEST_TID_MSG_SINK_HPP
