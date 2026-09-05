#include "gtest/gtest.h"
#include "logging/c/logging_c.h"

namespace logging_test {

class TestSyncLogger : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};

protected:
    LoggerSt *_logger{nullptr};
    SinkSt *_sink{nullptr};
};
}  // namespace logging_test
