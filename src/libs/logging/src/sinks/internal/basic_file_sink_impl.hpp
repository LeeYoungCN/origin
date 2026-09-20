
#ifndef ORIGIN_LOGGING_SINKS_INTERNAL_BASIC_FILE_SINK_IMPL_HPP
#define ORIGIN_LOGGING_SINKS_INTERNAL_BASIC_FILE_SINK_IMPL_HPP

#include <string>
#include <string_view>

#include "logging/log_msg.hpp"
#include "sinks/internal/sink_impl_base.hpp"
#include "utils/file_writer.h"

namespace origin::logging {
class BasicFileSinkImpl : public SinkImplBase {
public:
    BasicFileSinkImpl();
    ~BasicFileSinkImpl() override = default;
    explicit BasicFileSinkImpl(std::string_view file, bool overwrite = true);

    [[nodiscard]] const std::string& file() const;

protected:
    BasicFileSinkImpl(std::string_view file, bool overwrite, std::string_view paramStr);
    void log_it(const LogMsg& logMsg) override;
    void flush_it() override;
    void sink_it(std::string_view message);

    const std::string _file;
    const std::string _directory;
    const std::string _filename;
    const std::string _filenameStem;
    const std::string _extension;
    bool _overwrite{false};
    filesystem::FileWriter _fileWriter;
};

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_SINKS_INTERNAL_BASIC_FILE_SINK_IMPL_HPP
