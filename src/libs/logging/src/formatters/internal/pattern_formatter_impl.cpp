#include "formatters/internal/pattern_formatter_impl.h"

#include <format>
#include <iterator>
#include <memory>
#include <string>

#include "common/types/date_time_types.h"
#include "internal/common.h"
#include "logging/formatters/formatter.h"
#include "utils/date_time_utils.h"
#include "utils/filesystem_utils.h"

using namespace origin::date_time;
using namespace origin::filesystem;

namespace origin::logging {

PatternFormatterImpl::PatternFormatterImpl(std::string_view pattern) : _pattern(pattern)
{
    if (_pattern.empty()) {
        throw std::invalid_argument("pattern is empty.");
    }
}

void PatternFormatterImpl::format(const LogMsg& logMsg, std::string& content)
{
    constexpr uint32_t LOG_CONTENT_DEFAULT_LEN = 256;
    content.reserve(LOG_CONTENT_DEFAULT_LEN);
    bool needTrans = false;
    for (const char& c : _pattern) {
        if (needTrans) {
            log_msg_to_content(c, logMsg, content);
            needTrans = false;
        } else if (c == '%') {
            needTrans = true;
        } else {
            content.push_back(c);
        }
    }
}

void PatternFormatterImpl::format_time(const LogMsg& logMsg, std::string& logContent)
{
    DateTimeSt datetime = timestamp_to_date_time(logMsg.timestamp);
    std::format_to(std::back_insert_iterator(logContent),
                   "{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}",
                   datetime.year,
                   datetime.month,
                   datetime.day,
                   datetime.hour,
                   datetime.minute,
                   datetime.second,
                   datetime.millis);
}

void PatternFormatterImpl::log_msg_to_content(char symbol, const LogMsg& logMsg,
                                              std::string& logContent)
{
    switch (symbol) {
        case 'd':  // datetime
            format_time(logMsg, logContent);
            break;
        case 'n':  // logger name
            logContent.append(logMsg.loggerName);
            break;
        case 'l':  // lower level string
            logContent.append(log_level_to_string(logMsg.level, false));
            break;
        case 'L':  // upper level string
            logContent.append(log_level_to_string(logMsg.level));
            break;
        case 's':  // file name
            logContent.append(
                (logMsg.source.file.empty() ? "Filename" : get_filename(logMsg.source.file)));
            break;
        case 'g':  // file path
            logContent.append(logMsg.source.file.empty() ? "FilePath" : logMsg.source.file);
            break;
        case '#':  // lineNumber
            logContent.append(std::to_string(logMsg.source.line));
            break;
        case '!':  // function name
            logContent.append(logMsg.source.func.empty() ? "Function" : logMsg.source.func);
            break;
        case 't':  // thread id
            logContent.append(std::to_string(logMsg.threadId));
            break;
        case 'P':  // process id
            logContent.append(std::to_string(logMsg.procId));
            break;
        case 'v':  // message
            logContent.append(logMsg.data);
            break;
        case '%':
            logContent.push_back('%');
            break;
        default:
            logContent.push_back('%');
            logContent.push_back(symbol);
            break;
    }
}

std::unique_ptr<Formatter> PatternFormatterImpl::clone() const
{
    return std::make_unique<PatternFormatterImpl>(_pattern);
}
}  // namespace origin::logging
