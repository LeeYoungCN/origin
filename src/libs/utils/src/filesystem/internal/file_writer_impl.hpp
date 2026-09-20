#ifndef ORIGIN_UTILS_FILSYSTEM_INTERNAL_FILE_WRITER_IMPL_H
#define ORIGIN_UTILS_FILSYSTEM_INTERNAL_FILE_WRITER_IMPL_H
#include <fstream>
#include <ios>
#include <string>
#include <string_view>

#include "common/common_error_code.h"
#include "common/types/error_code_types.h"
#include "common/types/filesystem_types.h"

namespace origin::filesystem {

class FileWriterImpl {
public:
    FileWriterImpl() = delete;
    explicit FileWriterImpl(std::string_view file);
    ~FileWriterImpl() = default;

    ErrorCode open(bool overwrite = true);
    ErrorCode reopen(bool overwrite = true);
    void close();
    void write(std::string_view str);
    void write_line(std::string_view str);
    void flush();

    [[nodiscard]] FileSize size();
    [[nodiscard]] std::string filename_stem() const;
    [[nodiscard]] std::string filename() const;
    [[nodiscard]] std::string directory() const;
    [[nodiscard]] std::string full_path() const;
    [[nodiscard]] std::string extension() const;
    [[nodiscard]] ErrorCode get_last_error() const;

private:
    ErrorCode open_it(bool overwrite);
    [[nodiscard]] FileSize get_file_size_it();

private:
    std::string _file;
    std::ofstream _stream;
    std::ios::openmode _mode{(std::ios::out | std::ios::trunc)};
    ErrorCode _errcode{ERR_COMM_SUCCESS};
    FileSize _currSize{0};
};
}  // namespace origin::filesystem
#endif  // ORIGIN_UTILS_FILSYSTEM_INTERNAL_FILE_WRITER_IMPL_H
