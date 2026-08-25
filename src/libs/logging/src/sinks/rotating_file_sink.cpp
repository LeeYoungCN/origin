#include "logging/sinks/rotating_file_sink.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "sinks/internal/rotating_file_sink_impl.h"

namespace origin::logging {
using namespace origin::filesystem;

RotatingFileSink::RotatingFileSink() : SinkBase(std::make_unique<RotatingFileSinkImpl>()) {}

RotatingFileSink::RotatingFileSink(std::string_view file, bool rotateOnOpen)
    : SinkBase(std::make_unique<RotatingFileSinkImpl>(file, rotateOnOpen))
{
}

RotatingFileSink::RotatingFileSink(std::string_view file, uint32_t maxFileSize, uint32_t maxFiles,
                                   bool rotateOnOpen)
    : SinkBase(std::make_unique<RotatingFileSinkImpl>(file, maxFileSize, maxFiles, rotateOnOpen))
{
}

std::string RotatingFileSink::file() const
{
    throw_if_pimpl_null();
    return dynamic_cast<const RotatingFileSinkImpl *>(_pImpl.get())->file();
}

std::vector<std::string> RotatingFileSink::get_file_list() const
{
    throw_if_pimpl_null();
    return dynamic_cast<RotatingFileSinkImpl *>(_pImpl.get())->get_file_list();
}

void RotatingFileSink::set_max_file_size(uint32_t maxFileSize) const
{
    throw_if_pimpl_null();
    dynamic_cast<RotatingFileSinkImpl *>(_pImpl.get())->set_max_file_size(maxFileSize);
}

uint32_t RotatingFileSink::max_file_size() const
{
    throw_if_pimpl_null();
    return dynamic_cast<RotatingFileSinkImpl *>(_pImpl.get())->max_file_size();
}

void RotatingFileSink::set_max_files(uint32_t maxFiles) const
{
    throw_if_pimpl_null();
    return dynamic_cast<RotatingFileSinkImpl *>(_pImpl.get())->set_max_files(maxFiles);
}

uint32_t RotatingFileSink::max_files() const
{
    throw_if_pimpl_null();
    return dynamic_cast<RotatingFileSinkImpl *>(_pImpl.get())->max_files();
}

}  // namespace origin::logging
