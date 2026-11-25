#include "sinks/internal/basic_file_sink_impl.hpp"

#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

#include "origin/common/common_error_code.h"
#include "origin/common/debug/debug_logger.h"

#include "origin/filesystem/file_writer.h"
#include "origin/filesystem/filesystem.h"
#include "origin/utils/utils_error_code.h"

#include "origin/logging/log_msg.hpp"

#include "internal/common.hpp"
#include "sinks/internal/sink_impl_base.hpp"

namespace origin::logging {
using namespace origin::filesystem;

namespace {
std::string default_log_file()
{
    std::string file;
    if (const auto errc = get_default_log_file("log", file); errc != ERR_COMM_SUCCESS) {
        throw std::runtime_error("Failed to resolve default log file: " +
                                 std::string(get_utils_err_msg(errc)));
    }
    return file;
}
}  // namespace

BasicFileSinkImpl::BasicFileSinkImpl() : BasicFileSinkImpl(default_log_file(), true) {}

BasicFileSinkImpl::BasicFileSinkImpl(const std::string_view file, const bool overwrite)
    : BasicFileSinkImpl(
          file,
          overwrite,
          std::format(
              "BasicFileSinkImpl. file: \"{}\", mode: {}.", file, get_file_mode_str(overwrite)))
{
}

BasicFileSinkImpl::BasicFileSinkImpl(const std::string_view file,
                                     const bool overwrite,
                                     const std::string_view paramStr)
    : SinkImplBase(paramStr), _overwrite(overwrite), _fileWriter(file)
{
    _file = _fileWriter.full_path();
    if (_file.empty()) {
        const auto errc = _fileWriter.get_last_error();
        ORIGIN_DEBUG_ERR("Create BasicFileSinkImpl failed. File: \"{}\", mode: {}. msg: \"{}\".",
                         file,
                         get_file_mode_str(_overwrite),
                         get_utils_err_msg(errc));
        throw std::invalid_argument("Failed to resolve log file: " + std::string(file) +
                                    ", msg: " + std::string(get_utils_err_msg(errc)));
    }

    _directory = get_directory(_file);
    _filename = get_filename(_file);
    _filenameStem = get_filename_stem(_file);
    _extension = get_extension(_file);

    if (const auto errc = _fileWriter.open(_overwrite); errc != ERR_COMM_SUCCESS) {
        ORIGIN_DEBUG_ERR("Create BasicFileSinkImpl failed. File: \"{}\", mode: {}. msg: \"{}\".",
                         _file,
                         get_file_mode_str(_overwrite),
                         get_utils_err_msg(errc));

        throw std::runtime_error("Failed to open file: " + _file +
                                 ", msg: " + std::string(get_utils_err_msg(errc)));
    }
}

const std::string& BasicFileSinkImpl::file() const
{
    return _file;
}

void BasicFileSinkImpl::log_it(const LogMsg& logMsg)
{
    std::string content;
    _formatter->format(logMsg, content);
    sink_it(content);
}

void BasicFileSinkImpl::sink_it(const std::string_view message)
{
    if (const auto errc = _fileWriter.write_line(message); errc != ERR_COMM_SUCCESS) {
        throw std::runtime_error("Failed to write log: " + std::string(get_utils_err_msg(errc)));
    }
}

void BasicFileSinkImpl::flush_it()
{
    if (const auto errc = _fileWriter.flush(); errc != ERR_COMM_SUCCESS) {
        throw std::runtime_error("Failed to flush log: " + std::string(get_utils_err_msg(errc)));
    }
}

}  // namespace origin::logging
