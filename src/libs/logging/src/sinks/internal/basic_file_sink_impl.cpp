#include "sinks/internal/basic_file_sink_impl.h"

#include <format>
#include <stdexcept>

#include "common/debug/debug_logger.h"
#include "internal/common.h"
#include "utils/file_writer.h"
#include "utils/filesystem_utils.h"
#include "utils/utils_error_code.h"

namespace origin::logging {
using namespace origin::filesystem;

BasicFileSinkImpl::BasicFileSinkImpl() : BasicFileSinkImpl(get_default_log_file("log"), true) {}

BasicFileSinkImpl::BasicFileSinkImpl(std::string_view file, bool overwrite)
    : BasicFileSinkImpl(file, overwrite,
                        std::format("BasicFileSinkImpl. file: \"{}\", mode: {}.", file,
                                    get_file_mode_str(overwrite)))
{
}

BasicFileSinkImpl::BasicFileSinkImpl(std::string_view file, bool overwrite,
                                     std::string_view paramStr)
    : SinkImplBase(paramStr),
      _file(to_absolute_path(file)),
      _directory(get_directory(_file)),
      _filename(get_filename(_file)),
      _filenameStem(get_filename_stem(_file)),
      _extension(get_extension(_file)),
      _overwrite(overwrite),
      _fileWriter(_file)
{
    if (_file.empty()) {
        throw std::invalid_argument("file empty");
    }

    _fileWriter.open(_overwrite);
    if (_fileWriter.get_last_error() != ERR_COMM_SUCCESS) {
        DEBUG_LOGGER_ERR("Create BasicFileSinkImpl failed. File: \"{}\", mode: {}. msg: \"{}\".",
                         file,
                         get_file_mode_str(_overwrite),
                         get_utils_err_msg(_fileWriter.get_last_error()));

        throw std::runtime_error("Failed to open file: " + std::string(file));
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

void BasicFileSinkImpl::sink_it(std::string_view message)
{
    _fileWriter.write_line(message);
}

void BasicFileSinkImpl::flush_it()
{
    _fileWriter.flush();
}

}  // namespace origin::logging
