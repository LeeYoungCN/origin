#include <stdexcept>
#include <string>
#include <string_view>

#include "common/constants/filesystem_constants.h"
#include "common/types/filesystem_types.h"
#include "detail/common.hpp"
#include "gtest/gtest.h"
#include "logging/log_level.hpp"
#include "logging/log_msg.hpp"
#include "logging/log_source.hpp"
#include "logging/sinks/basic_file_sink.hpp"
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
    EXPECT_THROW(const BasicFileSink sink(""), std::invalid_argument);
}

TEST_F(TestBasicFileSink, create_file_invalid_path)
{
    create_dir(_dir);
    EXPECT_THROW(const BasicFileSink sink(_dir), std::runtime_error);
}

TEST_F(TestBasicFileSink, create_file_success)
{
    const std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    const BasicFileSink sink(logFile);
    EXPECT_TRUE(file_exists(logFile));
}

TEST_F(TestBasicFileSink, sink_log_level)
{
    const std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    BasicFileSink sink(logFile);
    EXPECT_TRUE(file_exists(logFile));

    for (const LogLevel level : LOG_LEVELS) {
        sink.set_level(level);
        EXPECT_EQ(sink.level(), level);
        for (const LogLevel log_level : LOG_LEVELS) {
            if (log_level == LogLevel::OFF) {
                EXPECT_FALSE(sink.should_log(log_level));
            } else if (log_level >= sink.level()) {
                EXPECT_TRUE(sink.should_log(log_level));
            } else {
                EXPECT_FALSE(sink.should_log(log_level));
            }
        }
    }
}

TEST_F(TestBasicFileSink, sink_log_and_flush)
{
    const std::string logFile = join_paths({_dir, get_logger_name(test_info_) + ".log"});
    BasicFileSink sink(logFile);
    EXPECT_TRUE(file_exists(logFile));
    auto fileSizeBefore = get_file_size(logFile);

    sink.set_pattern("[%d][%l]: %v");

    constexpr std::string_view message = "Test log message.";
    const std::string logContent = "[2024-01-01 12:00:00.000][E]: " + std::string(message);

    const FileSize messageSize = logContent.size() + LF_LENGTH;

    const LogMsg logMsg = create_log_msg(LOG_SRC_LOCAL, "logger", LogLevel::INFO, message);

    for (int i = 0; i < 100; ++i) {
        sink.log(logMsg);
        sink.flush();
        EXPECT_EQ(get_file_size(logFile), fileSizeBefore + messageSize);
        fileSizeBefore += messageSize;
    }
}

}  // namespace logging_test
