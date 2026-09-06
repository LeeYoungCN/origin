#include <format>
#include <string>

#include "detail/common.h"
#include "gtest/gtest.h"
#include "internal/common.h"
#include "internal/log_msg.h"
#include "logging/formatters/pattern_formatter.h"
#include "logging/log_level.h"
#include "utils/date_time_utils.h"
#include "utils/filesystem_utils.h"
#include "utils/process_utils.h"
#include "utils/thread_utils.h"

namespace logging_test {

using namespace origin::logging;
using namespace origin::thread;
using namespace origin::date_time;
using namespace origin::process;
using namespace origin::filesystem;

class TestPatternFormatter : public ::testing::Test {
protected:
    static constexpr std::string_view FORMATTER_DEFAULT_TIME_PATTERN = "%Y-%m-%d %H:%M:%S.%3f";
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {};
    void TearDown() override {};
};

/**
 * d: Date time;
 * n: Logger name;
 * l: Log level short name;
 * L: Log level full name;
 * s: File name;
 * g: File path;
 * #: Line number;
 * !: Function name;
 * t: Thread ID;
 * P: Process ID;
 * v: Log message;
 */

TEST_F(TestPatternFormatter, date_time)
{
    PatternFormatter formatter("%d");
    LogMsg msg;
    msg.timestamp = get_now_timestamp_ms();
    std::string content;
    formatter.format(msg, content);
    EXPECT_EQ(content, format_time_string(msg.timestamp, FORMATTER_DEFAULT_TIME_PATTERN));
}

TEST_F(TestPatternFormatter, logger_name)
{
    PatternFormatter formatter("%n");
    LogMsg msg;
    msg.loggerName = test_info_->name();
    std::string content;
    formatter.format(msg, content);
    EXPECT_EQ(content, test_info_->name());
}

TEST_F(TestPatternFormatter, abbr_log_level)
{
    PatternFormatter formatter("%l");
    LogMsg msg;
    for (LogLevel level : LOG_LEVELS) {
        msg.level = level;
        std::string content;
        formatter.format(msg, content);
        EXPECT_EQ(content, log_level_to_string(level, false));
    }
}

TEST_F(TestPatternFormatter, full_log_level)
{
    PatternFormatter formatter("%L");
    LogMsg msg;
    for (LogLevel level : LOG_LEVELS) {
        msg.level = level;
        std::string content;
        formatter.format(msg, content);
        EXPECT_EQ(content, log_level_to_string(level, true));
    }
}

TEST_F(TestPatternFormatter, file_name)
{
    PatternFormatter formatter("%s");
    LogMsg msg;
    msg.source.file = __FILE__;
    std::string content;
    formatter.format(msg, content);
    EXPECT_EQ(content, get_filename(__FILE__));
}

TEST_F(TestPatternFormatter, file_path)
{
    PatternFormatter formatter("%g");
    LogMsg msg;
    msg.source.file = __FILE__;
    std::string content;
    formatter.format(msg, content);
    EXPECT_EQ(content, __FILE__);
}

TEST_F(TestPatternFormatter, line_number)
{
    PatternFormatter formatter("%#");
    LogMsg msg;
    for (int i = 0; i < 100; i++) {
        msg.source.line = i;
        std::string content;
        formatter.format(msg, content);
        EXPECT_EQ(content, std::to_string(i));
    }
}

TEST_F(TestPatternFormatter, function_name)
{
    PatternFormatter formatter("%!");
    LogMsg msg;
    msg.source.func = __func__;
    std::string content;
    formatter.format(msg, content);
    EXPECT_EQ(content, __func__);
}

TEST_F(TestPatternFormatter, thread_id)
{
    PatternFormatter formatter("%t");
    LogMsg msg;
    msg.threadId = get_curr_thread_id();
    std::string content;
    formatter.format(msg, content);
    EXPECT_EQ(content, std::to_string(get_curr_thread_id()));
}

TEST_F(TestPatternFormatter, process_id)
{
    PatternFormatter formatter("%P");
    LogMsg msg;
    msg.procId = get_curr_proc_id();
    std::string content;
    formatter.format(msg, content);
    EXPECT_EQ(content, std::to_string(get_curr_proc_id()));
}

TEST_F(TestPatternFormatter, log_message)
{
    PatternFormatter formatter("%v");
    LogMsg msg;

    msg.data = test_info_->name();
    std::string content;
    formatter.format(msg, content);
    EXPECT_EQ(content, msg.data);
}

TEST_F(TestPatternFormatter, percent)
{
    PatternFormatter formatter("%%");
    LogMsg msg;

    std::string content;
    formatter.format(msg, content);
    EXPECT_EQ(content, "%");
}

TEST_F(TestPatternFormatter, default_pattern)
{
    PatternFormatter formatter;
    LogMsg msg(LOG_SRC_LOCAL, test_info_->name(), LogLevel::ERR, "test message");
    std::string content;
    formatter.format(msg, content);
    // "[%d][%L][%s:%#]: %v"
    std::string expect =
        std::format("[{}][{}][{}:{}]: {}",
                    format_time_string(msg.timestamp, FORMATTER_DEFAULT_TIME_PATTERN),
                    log_level_to_string(msg.level, false),
                    get_filename(msg.source.file),
                    msg.source.line,
                    msg.data);

    EXPECT_EQ(content, expect);
}

}  // namespace logging_test
