#pragma once
#ifndef ORIGIN_LOGGING_SINKS_INTERNAL_BASIC_FILE_SINK_IMPL_H
#define ORIGIN_LOGGING_SINKS_INTERNAL_BASIC_FILE_SINK_IMPL_H

#include <string>
#include <string_view>

#include "internal/log_msg.h"
#include "sinks/internal/sink_impl_base.h"
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

#endif  // ORIGIN_LOGGING_SINKS_INTERNAL_BASIC_FILE_SINK_IMPL_H
