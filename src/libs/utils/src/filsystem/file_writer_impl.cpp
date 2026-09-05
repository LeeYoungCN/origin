#include "file_writer_impl.h"

#include <fstream>
#include <stdexcept>
#include <string_view>

#include "common/common_error_code.h"
#include "common/constants/filesystem_constants.h"
#include "common/debug/debug_logger.h"
#include "internal/utils/filesystem_utils_internal.h"
#include "utils/date_time_utils.h"
#include "utils/filesystem_utils.h"
#include "utils/thread_utils.h"
#include "utils/utils_error_code.h"

#define MODE_STR(mode) ((mode) ? "overwrite" : "append")

namespace origin::filesystem {
using namespace origin::filesystem::internal;

FileWriterImpl::FileWriterImpl(std::string_view file) : _file(to_absolute_path(file))
{
    if (file.empty()) {
        _errcode = ERR_COMM_PARAM_EMPTY;
        ORIGIN_DEBUG_ERR("Create FileWriterImpl failed. File path is empty.");
        throw std::invalid_argument("Create FileWriterImpl failed. File path is empty.");
        return;
    }
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
        std::error_code ec(errno, std::generic_category());
        set_thread_last_err(ConvertSysEcToErrorCode(ec));
        ORIGIN_DEBUG_ERR("Open file failed. file: \"{}\", mode: {}, msg: {}",
                         _file.data(),
                         MODE_STR(overwrite),
                         get_thread_last_err_msg());
        _errcode = get_thread_last_err();
        return _errcode;
    }

    _currSize = get_file_size(_file);

    ORIGIN_DEBUG_TRACE(
        "Open file success. file: \"{}\", mode: {}.", _file.data(), MODE_STR(overwrite));
    _errcode = ERR_COMM_SUCCESS;
    set_thread_last_err(_errcode);
    return _errcode;
}

FileSize FileWriterImpl::get_file_size_it()
{
#if OS_WINDOWS
    return get_file_size(_file);
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
    if (!file_exists(_file) && get_thread_last_err() == ERR_UTILS_NOT_FILE) {
        ORIGIN_DEBUG_ERR("Write to text file failed. file: \"{}\". message: \"{}\".",
                         _file,
                         get_thread_last_err_msg());
        _errcode = get_thread_last_err();
        return _errcode;
    }

    if (!create_dir(get_directory(_file))) {
        _errcode = get_thread_last_err();
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
    set_thread_last_err(_errcode);
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
    set_thread_last_err(_errcode);
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
    set_thread_last_err(_errcode);
}

void FileWriterImpl::flush()
{
    _errcode = ERR_COMM_SUCCESS;
    _stream.flush();
    _currSize = get_file_size_it();
}

FileSize FileWriterImpl::size()
{
    return _stream.is_open() ? _currSize : get_file_size(_file);
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
