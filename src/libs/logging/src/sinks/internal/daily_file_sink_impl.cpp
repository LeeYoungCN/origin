#include "sinks/internal/daily_file_sink_impl.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "common/constants/date_time_constants.h"
#include "common/debug/debug_logger.h"
#include "common/types/date_time_types.h"
#include "internal/common.h"
#include "logging/sinks/daily_file_sink.h"
#include "sinks/internal/rotating_file_sink_impl_base.h"
#include "utils/date_time_utils.h"
#include "utils/file_writer.h"
#include "utils/filesystem_utils.h"

namespace origin::logging {
using namespace origin::filesystem;
using namespace origin::date_time;

constexpr char SPLIT_CHAR = '.';

DailyFileSinkImpl::DailyFileSinkImpl()
    : DailyFileSinkImpl(get_default_log_file(), DailyFileSink::DEFAULT_ROTATION_HOUR,
                        DailyFileSink::DEFAULT_ROTATION_MINUTE, DailyFileSink::DEFAULT_MAX_FILES,
                        false)
{
}

DailyFileSinkImpl::DailyFileSinkImpl(std::string_view file, bool overwrite)
    : DailyFileSinkImpl(file, DailyFileSink::DEFAULT_ROTATION_HOUR,
                        DailyFileSink::DEFAULT_ROTATION_MINUTE, DailyFileSink::DEFAULT_MAX_FILES,
                        overwrite)
{
}

DailyFileSinkImpl::DailyFileSinkImpl(std::string_view file, uint32_t hour, uint32_t minute,
                                     bool overwrite)
    : DailyFileSinkImpl(file, hour, minute, DailyFileSink::DEFAULT_MAX_FILES, overwrite)
{
}

DailyFileSinkImpl::DailyFileSinkImpl(std::string_view file, uint32_t hour, uint32_t minute,
                                     uint32_t maxFiles, bool overwrite)
    : RotatingFileSinkImplBase(
          file, overwrite, maxFiles, "daliy log file",
          std::format("DailyFileSinkImpl. file: \"{}\", hour: {}, minute: {}, maxFiles: {}.", file,
                      hour, minute, maxFiles)),
      _hour(hour),
      _minute(minute)
{
    if (file.empty()) {
        DEBUG_LOGGER_ERR("Create DailyFileSinkImpl failed. baseFile empty.");
        throw std::invalid_argument("baseFile empty.");
    }

    if (_hour < MIN_HOUR || _hour > MAX_HOUR) {
        DEBUG_LOGGER_ERR("Create DailyFileSinkImpl failed. hour invalid: {}.", _hour);
        throw std::out_of_range("hour out of range.");
    }

    if (_minute < MIN_MINUTE || _minute > MAX_MINUTE) {
        DEBUG_LOGGER_ERR("Create DailyFileSinkImpl failed. minute invalid: {}.", hour);
        throw std::out_of_range("minute out of range.");
    }

    if (maxFiles > DailyFileSink::MAX_FILES) {
        DEBUG_LOGGER_ERR("Create DailyFileSinkImpl failed. maxFiles out of range: {}.", maxFiles);
        throw std::out_of_range("maxFiles out of range.");
    }

    TimestampMs now = get_now_timestamp_ms();
    DateTimeSt dateTime = timestamp_to_date_time(now);
    dateTime.hour = _hour;
    dateTime.minute = _minute;
    dateTime.second = 0;
    dateTime.millis = 0;
    _rotateTime = date_time_to_timestamp(dateTime);

    if (_rotateTime < now) {
        _rotateTime += MILLIS_PER_DAY;
    }
    _fileTime = _rotateTime - MILLIS_PER_DAY;

    init_file_queue();
}

void DailyFileSinkImpl::set_max_files(uint32_t maxFiles)
{
    if (maxFiles <= DailyFileSink::MAX_FILES) {
        set_max_files_it(maxFiles);
    } else {
        DEBUG_LOGGER_ERR("maxFiles invalid: {}. maxFiles should be less than or equal to {}.",
                         maxFiles,
                         DailyFileSink::MAX_FILES);
    }
}
uint32_t DailyFileSinkImpl::max_files() const
{
    return max_files_it();
}

void DailyFileSinkImpl::log_it(const LogMsg& logMsg)
{
    if (logMsg.timestamp > _rotateTime) {
        if (_fileWriter.size() > 0) {
            std::string newFile = calc_log_file(_fileTime);
            rotate(newFile);
        }
        while (_rotateTime < logMsg.timestamp) {
            _rotateTime += MILLIS_PER_DAY;
        }
        _fileTime = _rotateTime - MILLIS_PER_DAY;
    }

    std::string content;
    _formatter->format(logMsg, content);
    sink_it(content);
}

TimestampMs DailyFileSinkImpl::parse_log_timestamp(std::string_view filename)
{
    constexpr uint32_t TIME_STR_LEN = 9;
    if (filename.size() != _filename.size() + TIME_STR_LEN) {
        return 0;
    }

    if (get_extension(filename) != _extension) {
        return 0;
    }

    uint32_t idx = 0;
    for (; idx < _filenameStem.size(); ++idx) {
        if (filename[idx] != _filenameStem[idx]) {
            return 0;
        }
    }

    if (filename[idx++] != SPLIT_CHAR) {
        return 0;
    }

    DateTimeSt dateTime;
    auto parse_number = [&](uint32_t len, uint32_t& number) -> bool {
        for (uint32_t i = 0; i < len; ++i) {
            char c = filename[idx++];
            if (c < '0' || c > '9') {
                return false;
            }
            number = number * 10 + static_cast<uint32_t>(c - '0');
        }
        return true;
    };

    TimestampMs timestamp = 0;
    if (parse_number(4, dateTime.year) && parse_number(2, dateTime.month) &&
        parse_number(2, dateTime.day)) {
        timestamp = date_time_to_timestamp(dateTime);
    }

    return timestamp;
}

std::string DailyFileSinkImpl::calc_log_file(TimestampMs time)
{
    auto dateTime = timestamp_to_date_time(time);
    auto filename = std::format("{}{}{:04}{:02}{:02}{}",
                                _filenameStem,
                                SPLIT_CHAR,
                                dateTime.year,
                                dateTime.month,
                                dateTime.day,
                                _extension);
    return join_paths({_directory, filename});
}

void DailyFileSinkImpl::init_file_queue()
{
    std::vector<std::pair<TimestampMs, std::string>> logList;
    for (const auto& entry : std::filesystem::directory_iterator(_directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        TimestampMs timestamp = parse_log_timestamp(entry.path().filename().string());
        if (timestamp > _fileTime) {
            continue;
        }

        std::string file = entry.path().string();
        if (calc_log_file(timestamp) == file) {
            logList.emplace_back(timestamp, file);
        }
    }

    std::sort(logList.begin(), logList.end());
    for (const auto& fileInfo : logList) {
        push_back_file(fileInfo.second);
        DEBUG_LOGGER_DBG("Find daily log file. file: \"{}\"", fileInfo.second);
    }
}

}  // namespace origin::logging
