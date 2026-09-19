#include "logging/sinks/basic_file_sink.hpp"

#include <memory>
#include <string>
#include <string_view>

#include "logging/sinks/sink_base.hpp"
#include "sinks/internal/basic_file_sink_impl.hpp"

namespace origin::logging {

BasicFileSink::BasicFileSink() : SinkBase(std::make_unique<BasicFileSinkImpl>()) {}

BasicFileSink::BasicFileSink(const std::string_view file, bool overwrite)
    : SinkBase(std::make_unique<BasicFileSinkImpl>(file, overwrite))
{
}

std::string BasicFileSink::file() const
{
    throw_if_pimpl_null();
    return dynamic_cast<const BasicFileSinkImpl *>(_pImpl.get())->file();
}

}  // namespace origin::logging
