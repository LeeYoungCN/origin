#include "sinks/internal/rotating_file_sink_impl.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <format>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "common/debug/debug_logger.h"
#include "internal/common.h"
#include "logging/log_msg.h"
#include "logging/sinks/rotating_file_sink.h"
#include "utils/date_time_utils.h"
#include "utils/file_writer.h"
#include "utils/filesystem_utils.h"

namespace origin::logging {
using namespace origin::filesystem;

namespace {
struct LogFileInfo {
    uint32_t idx{0};
    std::string file;
    TimestampMs modifyTime{0};
    FileSize fileSize{0};
    LogFileInfo(uint32_t i, std::string_view f)
        : idx(i), file(f), modifyTime(get_file_modify_time(f)), fileSize(get_file_size(f))
    {
    }

    [[nodiscard]] std::string to_string() const
    {
        return std::format(
            "idx: {}, time: [{}], size: {}, file: \"{}\"",
            idx,
            origin::date_time::format_time_string(modifyTime, "%Y-%m-%d %H:%M:%S.%3f"),
            fileSize,
            file);
    }
};
}  // namespace

RotatingFileSinkImpl::RotatingFileSinkImpl() : RotatingFileSinkImpl(get_default_log_file("log")) {}

RotatingFileSinkImpl::RotatingFileSinkImpl(std::string_view file, bool rotateOnOpen)
    : RotatingFileSinkImpl(file, RotatingFileSink::DEFAULT_MAX_FILE_SIZE,
                           RotatingFileSink::DEFAULT_MAX_FILES, rotateOnOpen)
{
}

RotatingFileSinkImpl::RotatingFileSinkImpl(std::string_view file, uint32_t maxFileSize,
                                           uint32_t maxFiles, bool rotateOnOpen)
    : RotatingFileSinkImplBase(
          file, false, maxFiles, "rotating log file",
          std::format("RotatingFileSinkImpl, File: \"{}\", MaxFileSize: {}, MaxFiles: {}.", file,
                      maxFileSize, maxFiles)),
      _maxFileSize(maxFileSize)
{
    if (maxFileSize == 0) {
        ORIGIN_DEBUG_ERR("Create RotatingFileSinkImpl failed. maxFileSize is 0.");
        throw std::invalid_argument("maxFileSize is 0.");
    }

    if (maxFiles > RotatingFileSink::MAX_FILES) {
        ORIGIN_DEBUG_ERR("Create RotatingFileSinkImpl failed. maxFiles out of range.");
        throw std::out_of_range("maxFiles out of range.");
    }

    init_file_queue();

    if (rotateOnOpen) {
        rotate(get_next_file());
    }
}

void RotatingFileSinkImpl::set_max_file_size(uint32_t maxFileSize)
{
    std::lock_guard const lock(_sinkMtx);
    if (maxFileSize > 0) {
        _maxFileSize.store(maxFileSize);
    } else {
        ORIGIN_DEBUG_ERR("maxFileSize invalid: {}.", maxFileSize);
    }
}

uint32_t RotatingFileSinkImpl::max_file_size() const
{
    return _maxFileSize.load();
}

void RotatingFileSinkImpl::set_max_files(uint32_t maxFiles)
{
    if (maxFiles <= RotatingFileSink::MAX_FILES) {
        set_max_files_it(maxFiles);
    } else {
        ORIGIN_DEBUG_ERR("maxFiles invalid: {}. maxFiles should be less than or equal to {}.",
                         maxFiles,
                         RotatingFileSink::MAX_FILES);
    }
}

uint32_t RotatingFileSinkImpl::max_files() const
{
    return max_files_it();
}

void RotatingFileSinkImpl::log_it(const LogMsg& logMsg)
{
    std::string content;
    _formatter->format(logMsg, content);

    if (content.size() + _fileWriter.size() > _maxFileSize.load()) {
        rotate(get_next_file());
    }
    sink_it(content);
}

void RotatingFileSinkImpl::init_file_queue()
{
    std::vector<LogFileInfo> logList;
    for (const auto& entry : std::filesystem::directory_iterator(_directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        uint32_t const idx = parse_log_index(entry.path().filename().string());

        if (idx < RotatingFileSink::MIN_INDEX || idx > RotatingFileSink::MAX_INDEX) {
            continue;
        }

        logList.emplace_back(idx, entry.path().string());
    }

    std::sort(
        logList.begin(), logList.end(), [](const LogFileInfo& a, const LogFileInfo& b) -> bool {
            if (a.modifyTime == b.modifyTime) {
                return a.idx < b.idx;
            } else {
                return a.modifyTime < b.modifyTime;
            };
        });

    set_next_idx(logList.empty() ? RotatingFileSink::MIN_INDEX : logList.back().idx + 1);
    for (const auto& logInfo : logList) {
        ORIGIN_DEBUG_DBG("Find rotating log file. {}", logInfo.to_string());
        push_back_file(logInfo.file);
    }
}

std::string RotatingFileSinkImpl::get_next_file()
{
    return _file + "." + std::to_string(get_next_idx());
}

void RotatingFileSinkImpl::set_next_idx(uint32_t idx)
{
    if (idx > RotatingFileSink::MAX_INDEX) {
        idx = RotatingFileSink::MIN_INDEX;
        ORIGIN_DEBUG_DBG("Rotate log file wrap around. nextIdx: {}.", _nextIdx);
    }
    _nextIdx = idx;
}

uint32_t RotatingFileSinkImpl::get_next_idx()
{
    uint32_t const idx = _nextIdx++;

    if (_nextIdx > RotatingFileSink::MAX_INDEX) {
        _nextIdx = RotatingFileSink::MIN_INDEX;
        ORIGIN_DEBUG_DBG("Rotate log file wrap around. nextIdx: {}.", _nextIdx);
    }
    return idx;
}

uint32_t RotatingFileSinkImpl::parse_log_index(std::string_view filename) const
{
    constexpr uint32_t MIN_SUFFIX_LEN = 2;  // .1
    constexpr uint32_t MAX_SUFFIX_LEN = 6;  // .20000

    if (filename.size() < _filename.size() + MIN_SUFFIX_LEN ||
        filename.size() > _filename.size() + MAX_SUFFIX_LEN) {
        return 0;
    }

    uint32_t i = 0;
    for (; i < _filename.length(); ++i) {
        if (filename.at(i) != _filename.at(i)) {
            return 0;
        }
    }

    if (filename.at(i++) != '.') {
        return 0;
    }

    if (filename.at(i) == '0') {
        return 0;
    }

    uint32_t idx = 0;
    for (; i < filename.size(); ++i) {
        auto c = filename.at(i);
        if (c >= '0' && c <= '9') {
            idx = idx * 10 + static_cast<uint32_t>(c - '0');
        } else {
            return 0;
        }
    }

    return idx;
}

}  // namespace origin::logging
