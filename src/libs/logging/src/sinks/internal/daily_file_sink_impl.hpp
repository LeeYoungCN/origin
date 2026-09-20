
#ifndef ORIGIN_LOGGING_SINKS_INTERNAL_DAILY_FILE_SINK_IMPL_HPP
#define ORIGIN_LOGGING_SINKS_INTERNAL_DAILY_FILE_SINK_IMPL_HPP
#include <cstdint>
#include <string>
#include <string_view>

#include "common/types/date_time_types.h"
#include "sinks/internal/rotating_file_sink_impl_base.hpp"

namespace origin::logging {
/**
 * @brief: 按照日期滚动日志。
 */
class DailyFileSinkImpl : public RotatingFileSinkImplBase {
public:
    DailyFileSinkImpl();
    explicit DailyFileSinkImpl(std::string_view file, bool overwrite = false);
    DailyFileSinkImpl(std::string_view file, uint32_t hour, uint32_t minute,
                      bool overwrite = false);
    DailyFileSinkImpl(std::string_view file, uint32_t hour, uint32_t minute, uint32_t maxFiles,
                      bool overwrite = false);
    ~DailyFileSinkImpl() override = default;

    void set_max_files(uint32_t maxFiles);
    [[nodiscard]] uint32_t max_files() const;

protected:
    void log_it(const LogMsg& logMsg) override;
    void init_file_queue() override;

private:
    TimestampMs parse_log_timestamp(std::string_view filename);
    std::string calc_log_file(TimestampMs time);

private:
    uint32_t _hour{0};
    uint32_t _minute{0};

    TimestampMs _fileTime{0};
    TimestampMs _rotateTime{0};
};

}  // namespace origin::logging

#endif  // ORIGIN_LOGGING_SINKS_INTERNAL_DAILY_FILE_SINK_IMPL_HPP
