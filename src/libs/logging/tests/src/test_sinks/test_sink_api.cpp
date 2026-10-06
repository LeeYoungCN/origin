#include <memory>
#include <stdexcept>
#include <string>

#include "detail/common.hpp"
#include "gtest/gtest.h"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/log_msg.hpp"
#include "logging/log_source.hpp"
#include "logging/sinks/basic_file_sink.hpp"
#include "logging/sinks/sink.hpp"
#include "utils/filesystem_utils.h"

using namespace origin::logging;
using namespace origin::filesystem;

namespace logging_test {
class TestSinkApi : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;
    void InitSink(const testing::TestInfo* test_info_);

    std::string _dir = get_log_dir();
    std::shared_ptr<BasicFileSink> _sink;
    std::string _name;
    std::string _file;
};

void TestSinkApi::InitSink(const testing::TestInfo* test_info_)
{
    _name = get_logger_name(test_info_);
    _file = join_paths({_dir, _name + ".log"});
    _sink = std::make_shared<BasicFileSink>(_file, true);
    _sink->set_level(LogLevel::TRACE);
    EXPECT_TRUE(file_exists(_file));
}

void TestSinkApi::SetUp()
{
    delete_dir(_dir, true);
}

void TestSinkApi::TearDown()
{
    delete_dir(_dir, true);
}

TEST_F(TestSinkApi, param_string)
{
    InitSink(test_info_);
    EXPECT_FALSE(_sink->param_string().empty());
}

TEST_F(TestSinkApi, set_pattern)
{
    InitSink(test_info_);

    const char* pattern = "[%d][%n][%L]: %v";
    _sink->set_pattern(pattern);

    LogMsg const logMsg = create_log_msg(LOG_SRC_LOCAL, _name, LogLevel::INFO, "Test message.");
    _sink->log(logMsg);
    _sink->flush();

    std::string expected;
    auto formatter = std::make_unique<PatternFormatter>(pattern);
    formatter->format(logMsg, expected);
    EXPECT_EQ(read_text_file(_file), expected + '\n');
}

TEST_F(TestSinkApi, set_pattern_failed_when_pattern_empty)
{
    InitSink(test_info_);
    EXPECT_THROW(_sink->set_pattern(""), std::invalid_argument);
}

TEST_F(TestSinkApi, set_formatter)
{
    InitSink(test_info_);

    auto formatter = std::make_unique<PatternFormatter>("[%d][%g:%#][%n][%l]: %v");
    _sink->set_formatter(formatter->clone());

    LogMsg const logMsg = create_log_msg(LOG_SRC_LOCAL, _name, LogLevel::INFO, "Test message.");
    _sink->log(logMsg);
    _sink->flush();

    std::string expected;
    formatter->format(logMsg, expected);
    EXPECT_EQ(read_text_file(_file), expected + '\n');
}

TEST_F(TestSinkApi, log_level_filter)
{
    InitSink(test_info_);

    for (const LogLevel filterLevel : LOG_LEVELS) {
        _sink->set_level(filterLevel);
        EXPECT_EQ(_sink->level(), filterLevel);
        for (const LogLevel level : LOG_LEVELS) {
            if (level == LogLevel::OFF) {
                EXPECT_FALSE(_sink->should_log(level));
            } else if (level >= _sink->level()) {
                EXPECT_TRUE(_sink->should_log(level));
            } else {
                EXPECT_FALSE(_sink->should_log(level));
            }
        }
    }
}

}  // namespace logging_test
