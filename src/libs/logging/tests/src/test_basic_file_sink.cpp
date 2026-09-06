#include <string>

#include "common/constants/filesystem_constants.h"
#include "detail/common.h"
#include "gtest/gtest.h"
#include "internal/log_msg.h"
#include "logging/sinks/basic_file_sink.h"
#include "utils/filesystem_utils.h"

using namespace origin::logging;
using namespace origin::filesystem;

namespace logging_test {
class TestBasicFileSink : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override;

protected:
    std::string _dir = get_log_dir();
};

void TestBasicFileSink::TearDown()
{
    delete_dir(_dir, true);
}

TEST_F(TestBasicFileSink, create_file_empty)
{
    EXPECT_THROW(BasicFileSink sink(""), std::invalid_argument);
}

TEST_F(TestBasicFileSink, create_file_invalid_path)
{
    create_dir(_dir);
    EXPECT_THROW(BasicFileSink sink(_dir), std::runtime_error);
}

TEST_F(TestBasicFileSink, create_file_success)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    BasicFileSink sink(logFile);
    EXPECT_TRUE(file_exists(logFile));
}

TEST_F(TestBasicFileSink, sink_log_level)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    BasicFileSink sink(logFile);
    EXPECT_TRUE(file_exists(logFile));

    for (LogLevel level : LOG_LEVELS) {
        sink.set_level(level);
        EXPECT_EQ(sink.level(), level);
        for (LogLevel level : LOG_LEVELS) {
            if (level == LogLevel::OFF) {
                EXPECT_FALSE(sink.should_log(level));
            } else if (level >= sink.level()) {
                EXPECT_TRUE(sink.should_log(level));
            } else {
                EXPECT_FALSE(sink.should_log(level));
            }
        }
    }
}

TEST_F(TestBasicFileSink, sink_log_and_flush)
{
    std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    BasicFileSink sink(logFile);
    EXPECT_TRUE(file_exists(logFile));
    auto fileSizeBefore = get_file_size(logFile);

    sink.set_pattern("[%d][%l]: %v");

    std::string_view message = "Test log message.";
    std::string logContent = "[2024-01-01 12:00:00.000][E]: " + std::string(message);

    FileSize messageSize = logContent.size() + LF_LENGTH;

    LogMsg logMsg(LOG_SRC_LOCAL, "logger", LogLevel::INFO, message);

    for (int i = 0; i < 100; ++i) {
        sink.log(logMsg);
        sink.flush();
        EXPECT_EQ(get_file_size(logFile), fileSizeBefore + messageSize);
        fileSizeBefore += messageSize;
    }
}

}  // namespace logging_test
