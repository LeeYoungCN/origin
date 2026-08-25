#include "sinks/internal/rotating_file_sink_impl_base.h"

#include <atomic>
#include <cstdint>
#include <deque>
#include <format>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "common/debug/debug_logger.h"
#include "internal/common.h"

namespace origin::logging {
using namespace origin::filesystem;
constexpr uint32_t DELETE_FILE_RETRY = 3;
constexpr uint32_t DELETE_FILE_SLEEP_MS = 10;
constexpr uint32_t RENAME_FILE_RETRY = 3;
constexpr uint32_t RENAME_FILE_SLEEP_MS = 10;

RotatingFileSinkImplBase::RotatingFileSinkImplBase(std::string_view file, bool overwrite,
                                                   uint32_t maxFiles, std::string_view itemName,
                                                   std::string_view paramStr)
    : BasicFileSinkImpl(file, overwrite, paramStr), _maxFiles(maxFiles), _itemName(itemName)
{
}

std::vector<std::string> RotatingFileSinkImplBase::get_file_list()
{
    std::lock_guard const lock(_sinkMtx);
    std::vector<std::string> rst;
    rst.reserve(_fileQue.size());
    for (auto& i : _fileQue) {
        rst.push_back(i);
    }
    return rst;
}

void RotatingFileSinkImplBase::set_max_files_it(uint32_t maxFiles)
{
    _maxFiles.store(maxFiles);
}

uint32_t RotatingFileSinkImplBase::max_files_it() const
{
    return _maxFiles.load();
}

void RotatingFileSinkImplBase::push_back_file(std::string_view file)
{
    _fileQue.emplace_back(file);
    DEBUG_LOGGER_TRACE("Enqueue {}. {}", _itemName, file);
}

void RotatingFileSinkImplBase::rotate(std::string_view newFile)
{
    _fileWriter.close();

    if (rename_file(file(), newFile, true, RENAME_FILE_RETRY, RENAME_FILE_SLEEP_MS)) {
        _fileWriter.reopen(true);
        push_back_file(newFile);
        DEBUG_LOGGER_DBG("Rotate log file success. {}", newFile);
    } else {
        _fileWriter.reopen(false);
    }

    delete_overflow_file();
}

void RotatingFileSinkImplBase::delete_overflow_file()
{
    // 保证剩余文件不超过最大文件数量，直到把能删除的都删了。
    while (_fileQue.size() > _maxFiles.load()) {
        auto file = _fileQue.front();
        if (!delete_file(file, DELETE_FILE_RETRY, DELETE_FILE_SLEEP_MS)) {
            DEBUG_LOGGER_ERR("Delete {} failed. {}", _itemName, file);
        } else {
            DEBUG_LOGGER_DBG("Delete {} success. {}", _itemName, file);
        }
        _fileQue.pop_front();
    }
}

}  // namespace origin::logging
