#include "utils/file_writer.h"

#include <memory>
#include <string>
#include <string_view>

#include "common/types/error_code_types.h"
#include "common/types/filesystem_types.h"
#include "filesystem/internal/file_writer_impl.hpp"

#define MODE_STR(mode) ((mode) ? "overwrite" : "append")

namespace origin::filesystem {

FileWriter::~FileWriter()
{
    if (_pimpl == nullptr) {
        return;
    }
    _pimpl.reset();
}

FileWriter::FileWriter(std::string_view file) : _pimpl(std::make_unique<FileWriterImpl>(file)) {}

ErrorCode FileWriter::open(bool overwrite)
{
    return _pimpl->open(overwrite);
}

ErrorCode FileWriter::reopen(bool overwrite)
{
    return _pimpl->reopen(overwrite);
}

void FileWriter::close()
{
    _pimpl->close();
}

void FileWriter::write(std::string_view str)
{
    _pimpl->write(str);
}

void FileWriter::write_line(std::string_view str)
{
    _pimpl->write_line(str);
}

void FileWriter::flush()
{
    _pimpl->flush();
}

FileSize FileWriter::size() const
{
    return _pimpl->size();
}

std::string FileWriter::filename_stem() const
{
    return _pimpl->filename_stem();
}

std::string FileWriter::filename() const
{
    return _pimpl->filename();
}

std::string FileWriter::directory() const
{
    return _pimpl->directory();
}

std::string FileWriter::full_path() const
{
    return _pimpl->full_path();
}

std::string FileWriter::extension() const
{
    return _pimpl->extension();
}

ErrorCode FileWriter::get_last_error() const
{
    return _pimpl->get_last_error();
}

}  // namespace origin::filesystem
