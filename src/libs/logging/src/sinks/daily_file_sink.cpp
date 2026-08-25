#include "logging/sinks/daily_file_sink.h"

#include <memory>
#include <string>
#include <string_view>

#include "sinks/internal/daily_file_sink_impl.h"

namespace origin::logging {

DailyFileSink::DailyFileSink() : SinkBase(std::make_unique<DailyFileSinkImpl>()) {}

DailyFileSink::DailyFileSink(std::string_view file, bool overwrite)
    : SinkBase(std::make_unique<DailyFileSinkImpl>(file, overwrite))
{
}

DailyFileSink::DailyFileSink(std::string_view file, uint32_t hour, uint32_t minute, bool overwrite)
    : SinkBase(std::make_unique<DailyFileSinkImpl>(file, hour, minute, overwrite))
{
}

DailyFileSink::DailyFileSink(std::string_view file, uint32_t hour, uint32_t minute,
                             uint32_t maxFiles, bool overwrite)
    : SinkBase(std::make_unique<DailyFileSinkImpl>(file, hour, minute, maxFiles, overwrite))

{
}

std::string DailyFileSink::file() const
{
    throw_if_pimpl_null();
    return dynamic_cast<const DailyFileSinkImpl *>(_pImpl.get())->file();
}

std::vector<std::string> DailyFileSink::get_file_list() const
{
    throw_if_pimpl_null();
    return dynamic_cast<DailyFileSinkImpl *>(_pImpl.get())->get_file_list();
}

void DailyFileSink::set_max_files(uint32_t maxFiles) const
{
    throw_if_pimpl_null();
    return dynamic_cast<DailyFileSinkImpl *>(_pImpl.get())->set_max_files(maxFiles);
}

uint32_t DailyFileSink::max_files() const
{
    throw_if_pimpl_null();
    return dynamic_cast<DailyFileSinkImpl *>(_pImpl.get())->max_files();
}

}  // namespace origin::logging
