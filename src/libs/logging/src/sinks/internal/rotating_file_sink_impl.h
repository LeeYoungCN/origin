#ifndef ORIGIN_LOGGING_SINKS_INTERNAL_ROTATING_FILE_SINK_IMPL_H
#define ORIGIN_LOGGING_SINKS_INTERNAL_ROTATING_FILE_SINK_IMPL_H

#include <atomic>
#include <cstdint>
#include <string_view>

#include "sinks/internal/rotating_file_sink_impl_base.h"

namespace origin::logging {
class RotatingFileSinkImpl : public RotatingFileSinkImplBase {
public:
    RotatingFileSinkImpl();
    ~RotatingFileSinkImpl() override = default;
    explicit RotatingFileSinkImpl(std::string_view file, bool rotateOnOpen = false);
    RotatingFileSinkImpl(std::string_view file, uint32_t maxFileSize, uint32_t maxFiles,
                         bool rotateOnOpen = false);

public:
    void set_max_file_size(uint32_t maxFileSize);
    [[nodiscard]] uint32_t max_file_size() const;

    void set_max_files(uint32_t maxFiles);
    [[nodiscard]] uint32_t max_files() const;

protected:
    void log_it(const LogMsg& logMsg) override;

private:
    std::string get_next_file();
    uint32_t get_next_idx();
    void set_next_idx(uint32_t idx);
    uint32_t parse_log_index(std::string_view file) const;
    void init_file_queue() override;

private:
    std::atomic<uint32_t> _maxFileSize{0};
    uint32_t _nextIdx{0};
};
}  // namespace origin::logging
#endif  // ORIGIN_LOGGING_SINKS_INTERNAL_ROTATING_FILE_SINK_IMPL_H
