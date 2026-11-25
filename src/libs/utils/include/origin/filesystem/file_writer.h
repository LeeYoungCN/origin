#ifndef ORIGIN_FILESYSTEM_FILE_WRITER_H
#define ORIGIN_FILESYSTEM_FILE_WRITER_H

#include <memory>
#include <string>
#include <string_view>

#include "origin/common/types/error_code_types.h"
#include "origin/common/types/filesystem_types.h"

namespace origin::filesystem {
class FileWriterImpl;
class FileWriter {
public:
    FileWriter() = delete;
    explicit FileWriter(std::string_view file);
    ~FileWriter();

    [[nodiscard]] ErrorCode open(bool overwrite = true);
    [[nodiscard]] ErrorCode reopen(bool overwrite = true);
    void close();
    [[nodiscard]] ErrorCode write(std::string_view str);
    [[nodiscard]] ErrorCode write_line(std::string_view str);
    [[nodiscard]] ErrorCode flush();

    [[nodiscard]] FileSize size() const;
    [[nodiscard]] std::string filename_stem() const;
    [[nodiscard]] std::string filename() const;
    [[nodiscard]] std::string directory() const;
    [[nodiscard]] std::string full_path() const;
    [[nodiscard]] std::string extension() const;
    // 兼容旧用法；新代码应直接检查 write/write_line/flush 的返回值。
    [[nodiscard]] ErrorCode get_last_error() const;

private:
    std::unique_ptr<FileWriterImpl> _pimpl;
};

}  // namespace origin::filesystem

#endif  // ORIGIN_FILESYSTEM_FILE_WRITER_H
