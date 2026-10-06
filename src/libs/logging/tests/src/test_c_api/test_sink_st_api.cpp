#include <memory>
#include <string>

#include "detail/common.hpp"
#include "gtest/gtest.h"
#include "logging/c/logging_c.h"
#include "logging/formatters/pattern_formatter.hpp"
#include "logging/log_level.hpp"
#include "logging/log_msg.hpp"
#include "logging/log_source.hpp"
#include "logging/sinks/sink.hpp"
#include "utils/filesystem_utils.h"

using namespace origin::logging;
using namespace origin::filesystem;

namespace logging_test {
class TestSinkStApi : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override;
    void TearDown() override;
    void InitSink(const testing::TestInfo* test_info_);

    std::string _dir = get_log_dir();
    SinkSt* _nullSink = create_mock_sink_st(nullptr);
    LoggerSt* _logger{nullptr};
    SinkSt* _sink{nullptr};
    std::string _name;
    std::string _file;
    const char* _pattern = "[%s:%!][%g:%#][%n][%L][Tid: %t][%l]Pid: %P]: %v";
};

void TestSinkStApi::InitSink(const testing::TestInfo* test_info_)
{
    _name = get_logger_name(test_info_);
    _file = join_paths({_dir, _name + ".log"});
    _sink = origin_create_basic_file_sink(_file.c_str(), true);
    _logger = origin_create_sync_logger(_name.c_str(), &_sink, 1);
    origin_logger_set_level(_logger, LOG_LEVEL_C_TRACE);
    origin_sink_set_level(_sink, LOG_LEVEL_C_TRACE);
    EXPECT_TRUE(file_exists(_file));
}

void TestSinkStApi::SetUp()
{
    delete_dir(_dir, true);
}

void TestSinkStApi::TearDown()
{
    origin_destroy_logger(_logger);
    origin_destroy_sink(_sink);
    destroy_mock_sink_st(_nullSink);
    delete_dir(_dir, true);
}

TEST_F(TestSinkStApi, set_pattern)
{
    InitSink(test_info_);

    origin_sink_set_pattern(_sink, _pattern);
    LogMsg const logMsg = create_log_msg(LOG_SRC_LOCAL, _name, LogLevel::INFO, "Test message.");
    origin_logger_log(_logger,
                      logMsg.source.file.c_str(),
                      logMsg.source.line,
                      logMsg.source.func.c_str(),
                      LOG_LEVEL_C_INFO,
                      "%s",
                      logMsg.data.c_str());
    origin_logger_flush(_logger);

    std::string expected;
    auto formatter = std::make_unique<PatternFormatter>(_pattern);
    formatter->format(logMsg, expected);
    EXPECT_EQ(read_text_file(_file), expected + '\n');
}

TEST_F(TestSinkStApi, set_pattern_failed_when_pattern_empty)
{
    InitSink(test_info_);
    origin_sink_set_pattern(_sink, "");
}

TEST_F(TestSinkStApi, set_pattern_failed_when_pattern_nullptr)
{
    InitSink(test_info_);
    origin_sink_set_pattern(_sink, nullptr);
}

TEST_F(TestSinkStApi, set_pattern_failed_when_sink_nullptr)
{
    origin_sink_set_pattern(nullptr, _pattern);
    origin_sink_set_pattern(_nullSink, _pattern);
}

TEST_F(TestSinkStApi, set_formatter)
{
    InitSink(test_info_);

    auto formatterSt = origin_create_pattern_formatter(_pattern);
    origin_sink_set_formatter(_sink, formatterSt);
    origin_destroy_formatter(formatterSt);

    LogMsg const logMsg = create_log_msg(LOG_SRC_LOCAL, _name, LogLevel::INFO, "Test message.");
    origin_logger_log(_logger,
                      logMsg.source.file.c_str(),
                      logMsg.source.line,
                      logMsg.source.func.c_str(),
                      LOG_LEVEL_C_INFO,
                      "%s",
                      logMsg.data.c_str());
    origin_logger_flush(_logger);

    std::string expected;
    auto formatter = std::make_unique<PatternFormatter>(_pattern);
    formatter->format(logMsg, expected);
    EXPECT_EQ(read_text_file(_file), expected + '\n');
}

TEST_F(TestSinkStApi, set_formatter_failed_when_sink_nullptr)
{
    InitSink(test_info_);
    auto formatterSt = origin_create_pattern_formatter(_pattern);
    origin_sink_set_formatter(nullptr, formatterSt);
    origin_sink_set_formatter(_nullSink, formatterSt);
    origin_destroy_formatter(formatterSt);
}

TEST_F(TestSinkStApi, set_formatter_failed_when_formatter_nullptr)
{
    InitSink(test_info_);
    origin_sink_set_formatter(_sink, nullptr);
    auto nullFormatter = origin_create_pattern_formatter(nullptr);
    origin_sink_set_formatter(_sink, nullFormatter);
    origin_destroy_formatter(nullFormatter);
}

TEST_F(TestSinkStApi, log_level_filter)
{
    InitSink(test_info_);

    for (const LogLevelC filterLevel : C_LOG_LEVELS) {
        origin_sink_set_level(_sink, filterLevel);
        EXPECT_EQ(origin_sink_level(_sink), filterLevel);
        for (const LogLevelC level : C_LOG_LEVELS) {
            if (level == LOG_LEVEL_C_OFF) {
                EXPECT_FALSE(origin_sink_should_log(_sink, level));
            } else if (level >= origin_sink_level(_sink)) {
                EXPECT_TRUE(origin_sink_should_log(_sink, level));
            } else {
                EXPECT_FALSE(origin_sink_should_log(_sink, level));
            }
        }
    }
}

TEST_F(TestSinkStApi, set_level_failed_when_sink_nullptr)
{
    InitSink(test_info_);
    origin_sink_set_level(nullptr, LOG_LEVEL_C_INFO);
    origin_sink_set_level(_nullSink, LOG_LEVEL_C_INFO);
}

TEST_F(TestSinkStApi, set_level_failed_when_level_invalid)
{
    InitSink(test_info_);
    origin_sink_set_level(_sink, INVALID_LEVEL_C);
}

TEST_F(TestSinkStApi, should_log_failed_when_sink_nullptr)
{
    InitSink(test_info_);
    EXPECT_FALSE(origin_sink_should_log(nullptr, LOG_LEVEL_C_INFO));
    EXPECT_FALSE(origin_sink_should_log(_nullSink, LOG_LEVEL_C_INFO));
}

TEST_F(TestSinkStApi, level_failed_when_sink_nullptr)
{
    InitSink(test_info_);
    EXPECT_EQ(origin_sink_level(nullptr), LOG_LEVEL_C_OFF);
    EXPECT_EQ(origin_sink_level(_nullSink), LOG_LEVEL_C_OFF);
}

}  // namespace logging_test
