#include "filesystem/internal/file_writer_impl.hpp"

#include <cerrno>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include "origin/common/common_error_code.h"
#include "origin/common/constants/filesystem_constants.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/common/macros/compiler.h"
#include "origin/common/types/error_code_types.h"
#include "origin/common/types/filesystem_types.h"

#include "origin/utils/utils_error_code.h"

#include "filesystem/internal/common.hpp"
#include "origin/date_time/date_time.h"
#include "origin/filesystem/filesystem.h"

#define MODE_STR(mode) ((mode) ? "overwrite" : "append")

namespace origin::filesystem {

FileWriterImpl::FileWriterImpl(std::string_view file) : _file()
{
    if (file.empty()) {
        _errcode = ERR_COMM_PARAM_EMPTY;
        ORIGIN_DEBUG_ERR("Create FileWriterImpl failed. File path is empty.");
        throw std::invalid_argument("Create FileWriterImpl failed. File path is empty.");
        return;
    }
    (void)to_absolute_path(file, "", _file);
}

ErrorCode FileWriterImpl::open_it(bool overwrite)
{
    close();
    if (overwrite) {
        _mode = (std::ios::out | std::ios::trunc);
    } else {
        _mode = (std::ios::out | std::ios::app);
    }

    _stream = std::ofstream(_file, _mode);

    date_time::sleep_ms(origin::filesystem::FILE_OPEN_INTERVAL_MS);

    if (!_stream.is_open()) {
        std::error_code const ec(errno, std::generic_category());
        _errcode = ConvertSysEcToErrorCode(ec);
        ORIGIN_DEBUG_ERR("Open file failed. file: \"{}\", mode: {}, msg: {}",
                         _file.data(),
                         MODE_STR(overwrite),
                         get_utils_err_msg(_errcode));
        return _errcode;
    }

    FileSize size = 0;
    _errcode = get_file_size(_file, size);
    if (_errcode != ERR_COMM_SUCCESS) {
        return _errcode;
    }
    _currSize = size;

    ORIGIN_DEBUG_TRACE(
        "Open file success. file: \"{}\", mode: {}.", _file.data(), MODE_STR(overwrite));
    _errcode = ERR_COMM_SUCCESS;
    return _errcode;
}

FileSize FileWriterImpl::get_file_size_it()
{
#if OS_WINDOWS
    FileSize size = 0;
    if (get_file_size(_file, size) != ERR_COMM_SUCCESS) {
        return 0;
    }
    return size;
#else
    // tellp() returns a signed position type (std::streampos); handle possible -1 and avoid
    // implicit conversion to unsigned size_t which would change signedness.
    auto pos = _stream.tellp();
    if (pos == std::ofstream::pos_type(-1)) {
        return 0;
    } else {
        return static_cast<FileSize>(pos);
    }
#endif
}

ErrorCode FileWriterImpl::open(bool overwrite)
{
    close();
    EntryType type = EntryType::UNKNOWN;
    (void)get_entry_type(_file, type);
    if (type != EntryType::FILE && type != EntryType::NONEXISTENT) {
        _errcode = ERR_UTILS_NOT_FILE;
        ORIGIN_DEBUG_ERR("Write to text file failed. file: \"{}\". message: \"{}\".",
                         _file,
                         get_utils_err_msg(_errcode));
        return _errcode;
    }

    _errcode = create_dir(get_directory(_file));
    if (_errcode != ERR_COMM_SUCCESS) {
        return _errcode;
    }

    return open_it(overwrite);
}

ErrorCode FileWriterImpl::reopen(bool overwrite)
{
    return open_it(overwrite);
}

void FileWriterImpl::close()
{
    _errcode = ERR_COMM_SUCCESS;
    if (_stream.is_open()) {
        _stream.flush();
        _stream.close();
        _currSize = 0;
        ORIGIN_DEBUG_TRACE("Close file success. file: \"{}\".", _file.data());
    }
}

void FileWriterImpl::write(std::string_view str)
{
    if (!_stream.is_open()) {
        _errcode = ERR_UTILS_FILE_NOT_OPEN;
        ORIGIN_DEBUG_ERR(
            "Write failed. file: \"{}\", msg: {}.", _file.c_str(), get_utils_err_msg(_errcode));
    } else {
        _errcode = ERR_COMM_SUCCESS;
        _stream << str;
        _currSize += str.length();
    }
}

void FileWriterImpl::write_line(std::string_view str)
{
    if (!_stream.is_open()) {
        _errcode = ERR_UTILS_FILE_NOT_OPEN;
        ORIGIN_DEBUG_ERR("Write line failed. file: \"{}\", msg: {}.",
                         _file.c_str(),
                         get_utils_err_msg(_errcode));
    } else {
        _errcode = ERR_COMM_SUCCESS;
        _stream << str << '\n';
        _currSize += str.length() + origin::filesystem::LF_LENGTH;
    }
}

void FileWriterImpl::flush()
{
    _errcode = ERR_COMM_SUCCESS;
    _stream.flush();
    _currSize = get_file_size_it();
}

FileSize FileWriterImpl::size()
{
    if (_stream.is_open()) {
        return _currSize;
    }
    FileSize size = 0;
    (void)get_file_size(_file, size);
    return size;
}

std::string FileWriterImpl::filename_stem() const
{
    return get_filename_stem(_file);
}

std::string FileWriterImpl::filename() const
{
    return get_filename(_file);
}

std::string FileWriterImpl::directory() const
{
    return get_directory(_file);
}

std::string FileWriterImpl::full_path() const
{
    return _file;
}

std::string FileWriterImpl::extension() const
{
    return get_extension(_file);
}

ErrorCode FileWriterImpl::get_last_error() const
{
    return _errcode;
}

}  // namespace origin::filesystem
