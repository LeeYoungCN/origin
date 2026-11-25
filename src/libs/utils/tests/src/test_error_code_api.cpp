#include <gtest/gtest.h>

#include <origin/common/common_error_code.h>
#include <origin/date_time/date_time.h>
#include <origin/filesystem/filesystem.h>
#include <origin/process/process.h>
#include <origin/strings/strings.h>
#include <origin/thread/thread.h>
#include <origin/utils/utils_error_code.h>

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include "origin/common/constants/date_time_constants.h"
#include "origin/common/types/date_time_types.h"
#include "origin/common/types/filesystem_types.h"

namespace {

using namespace origin::date_time;
using namespace origin::filesystem;

std::string FormatViaVaList(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const std::string result = origin::strings::va_list_to_string(format, args);
    va_end(args);
    return result;
}

TEST(FilesystemErrorCodeApi, ReadAndQuery)
{
    std::string cwd;
    ASSERT_EQ(get_curr_working_dir(cwd), ERR_COMM_SUCCESS);
    ASSERT_FALSE(cwd.empty());

    std::string dir;
    ASSERT_EQ(to_absolute_path("origin_ec_test_dir", cwd, dir), ERR_COMM_SUCCESS);
    (void)delete_dir(dir, true);
    ASSERT_EQ(create_dir(dir, true), ERR_COMM_SUCCESS);

    std::string file;
    ASSERT_EQ(join_paths({dir, "a.txt"}, file), ERR_COMM_SUCCESS);
    ASSERT_EQ(create_file(file), ERR_COMM_SUCCESS);
    ASSERT_EQ(write_text_file(file, "hello", true), ERR_COMM_SUCCESS);

    std::string text;
    ASSERT_EQ(read_text_file(file, text), ERR_COMM_SUCCESS);
    EXPECT_EQ(text, "hello");

    FileSize size = 0;
    ASSERT_EQ(get_file_size(file, size), ERR_COMM_SUCCESS);
    EXPECT_EQ(size, 5u);

    FileInfo info{};
    ASSERT_EQ(get_file_info(file, info), ERR_COMM_SUCCESS);
    EXPECT_EQ(info.type, EntryType::FILE);
    EXPECT_EQ(info.size, 5u);
    EXPECT_GT(info.modifyTime, 0);

    TimestampMs modifyTime = 0;
    ASSERT_EQ(get_file_modify_time(file, modifyTime), ERR_COMM_SUCCESS);
    EXPECT_GT(modifyTime, 0);

    std::vector<uint8_t> binary;
    ASSERT_EQ(read_binary_file(file, binary), ERR_COMM_SUCCESS);
    ASSERT_EQ(binary.size(), 5u);
    EXPECT_EQ(binary[0], static_cast<uint8_t>('h'));

    EntryType type = EntryType::UNKNOWN;
    ASSERT_EQ(get_entry_type(dir, type), ERR_COMM_SUCCESS);
    EXPECT_EQ(type, EntryType::DIRECTORY);

    std::string normalized;
    ASSERT_EQ(normalize_path("a/./b/../c", normalized), ERR_COMM_SUCCESS);
    std::string expectedJoined;
    ASSERT_EQ(join_paths({"a", "c"}, expectedJoined), ERR_COMM_SUCCESS);
    EXPECT_EQ(normalized, expectedJoined);

    ASSERT_EQ(delete_dir(dir, true), ERR_COMM_SUCCESS);
}

TEST(FilesystemErrorCodeApi, MissingPath)
{
    const std::string missing = "origin_ec_test_missing_file";

    std::string text;
    EXPECT_EQ(read_text_file(missing, text), ERR_UTILS_NOT_FOUND);

    FileSize size = 0;
    EXPECT_EQ(get_file_size(missing, size), ERR_UTILS_NOT_FOUND);

    EntryType type = EntryType::UNKNOWN;
    EXPECT_EQ(get_entry_type(missing, type), ERR_UTILS_NOT_FOUND);
    EXPECT_EQ(type, EntryType::NONEXISTENT);
}

TEST(FilesystemErrorCodeApi, MissingPathUnderFileComponent)
{
    std::string cwd;
    ASSERT_EQ(get_curr_working_dir(cwd), ERR_COMM_SUCCESS);

    std::string dir;
    ASSERT_EQ(to_absolute_path("origin_ec_test_under_file", cwd, dir), ERR_COMM_SUCCESS);
    (void)delete_dir(dir, true);
    ASSERT_EQ(create_dir(dir, true), ERR_COMM_SUCCESS);

    std::string file;
    ASSERT_EQ(join_paths({dir, "file.txt"}, file), ERR_COMM_SUCCESS);
    ASSERT_EQ(create_file(file), ERR_COMM_SUCCESS);

    // 路径中间一段是普通文件，任何子项都不可能存在；应与 ENOENT 一样按不存在处理。
    const std::string missing = file + "/missing";
    EntryType type = EntryType::UNKNOWN;
    EXPECT_EQ(get_entry_type(missing, type), ERR_UTILS_NOT_FOUND);
    EXPECT_EQ(type, EntryType::NONEXISTENT);
    EXPECT_EQ(delete_dir(missing), ERR_COMM_SUCCESS);

    EXPECT_EQ(delete_dir(dir, true), ERR_COMM_SUCCESS);
}

TEST(FilesystemErrorCodeApi, SymlinkAndSpecialFiles)
{
    std::string cwd;
    ASSERT_EQ(get_curr_working_dir(cwd), ERR_COMM_SUCCESS);

    std::string dir;
    ASSERT_EQ(to_absolute_path("origin_ec_test_special_dir", cwd, dir), ERR_COMM_SUCCESS);
    (void)delete_dir(dir, true);
    ASSERT_EQ(create_dir(dir, true), ERR_COMM_SUCCESS);

    std::string target;
    ASSERT_EQ(join_paths({dir, "target.txt"}, target), ERR_COMM_SUCCESS);
    ASSERT_EQ(create_file(target), ERR_COMM_SUCCESS);

    std::string link;
    ASSERT_EQ(join_paths({dir, "link"}, link), ERR_COMM_SUCCESS);
    std::error_code ec;
    std::filesystem::create_symlink(target, link, ec);
    if (!ec) {
        EntryType type = EntryType::UNKNOWN;
        EXPECT_EQ(get_entry_type(link, type), ERR_COMM_SUCCESS);
        EXPECT_EQ(type, EntryType::SYMBOL_LINK);
    }

#ifndef _WIN32
    std::string fifo;
    ASSERT_EQ(join_paths({dir, "fifo"}, fifo), ERR_COMM_SUCCESS);
    if (mkfifo(fifo.c_str(), 0600) == 0) {
        EntryType fifoType = EntryType::UNKNOWN;
        EXPECT_EQ(get_entry_type(fifo, fifoType), ERR_COMM_SUCCESS);
        EXPECT_EQ(fifoType, EntryType::FIFO);
    } else {
        // 某些文件系统（如 WSL 的 /mnt 挂载）不支持 FIFO；不影响后续 socket 检查。
        GTEST_LOG_(INFO) << "mkfifo not supported at " << fifo << ", skipping FIFO assertion";
    }

    const int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);
    // UNIX socket 的 sun_path 只有约 108 字节，不能复用构建目录下的长路径；
    // 放到系统临时目录并带 pid，避免并行测试互相覆盖。
    const std::string socketPath =
        (std::filesystem::temp_directory_path() /
         ("origin_ec_sock_" + std::to_string(static_cast<long long>(getpid()))))
            .string();
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    ASSERT_LT(socketPath.size(), sizeof(address.sun_path));
    std::memcpy(address.sun_path, socketPath.c_str(), socketPath.size() + 1);
    ASSERT_EQ(bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)), 0);
    close(fd);

    EntryType socketType = EntryType::UNKNOWN;
    EXPECT_EQ(get_entry_type(socketPath, socketType), ERR_COMM_SUCCESS);
    EXPECT_EQ(socketType, EntryType::SOCKET);
    (void)unlink(socketPath.c_str());
#endif

    (void)delete_dir(dir, true);
}

TEST(DateTimeErrorCodeApi, GetNowDateTime)
{
    DateTimeSt now{};
    ASSERT_EQ(get_now_date_time(now), ERR_COMM_SUCCESS);
    EXPECT_GT(now.year, 1970u);

    const DateTimeSt value = get_now_date_time();
    EXPECT_GT(value.year, 1970u);
}

TEST(DateTimeErrorCodeApi, Conversions)
{
    DateTimeSt utc{};
    ASSERT_EQ(utc_date_time(0, utc), ERR_COMM_SUCCESS);
    EXPECT_EQ(utc.year, 1970u);
    EXPECT_EQ(utc.month, 1u);
    EXPECT_EQ(utc.day, 1u);

    DateTimeSt local{};
    ASSERT_EQ(local_date_time(0, local), ERR_COMM_SUCCESS);
    EXPECT_EQ(local.year, 1970u);

    DateTimeSt viaZone{};
    ASSERT_EQ(timestamp_to_date_time(0, TimeZone::UTC, viaZone), ERR_COMM_SUCCESS);
    EXPECT_EQ(viaZone.year, 1970u);

    // date_time_to_timestamp 按本地时区解释，用往返验证
    constexpr TimestampMs knownTimestamp = 1234567890000;
    DateTimeSt known{};
    ASSERT_EQ(local_date_time(knownTimestamp, known), ERR_COMM_SUCCESS);
    TimestampMs roundTrip = 0;
    ASSERT_EQ(date_time_to_timestamp(known, roundTrip), ERR_COMM_SUCCESS);
    EXPECT_EQ(roundTrip, knownTimestamp);
}

TEST(DateTimeErrorCodeApi, NamesAndFormatting)
{
    std::string_view name;
    EXPECT_EQ(get_month_full_name(0, name), ERR_UTILS_MONTH_INVALID);
    ASSERT_EQ(get_month_full_name(1, name), ERR_COMM_SUCCESS);
    EXPECT_EQ(name, "January");

    EXPECT_EQ(get_weekday_full_name(9, name), ERR_UTILS_WEEKDAY_INVALID);
    ASSERT_EQ(get_weekday_abbr_name(1, name), ERR_COMM_SUCCESS);
    EXPECT_EQ(name, "Mon");

    DateTimeSt utc{};
    ASSERT_EQ(utc_date_time(0, utc), ERR_COMM_SUCCESS);

    std::string text;
    ASSERT_EQ(format_time_string(utc, "%Y-%m-%d", text), ERR_COMM_SUCCESS);
    EXPECT_EQ(text, "1970-01-01");

    std::string appended;
    ASSERT_EQ(append_time_string(appended, utc, "%Y"), ERR_COMM_SUCCESS);
    EXPECT_EQ(appended, "1970");

    char buffer[16] = {};
    size_t length = 0;
    ASSERT_EQ(format_time_buffer(buffer, sizeof(buffer), utc, "%Y", length), ERR_COMM_SUCCESS);
    EXPECT_EQ(length, 4u);
    EXPECT_STREQ(buffer, "1970");

    char tinyBuffer[2] = {};
    size_t smallLength = 0;
    EXPECT_EQ(format_time_buffer(tinyBuffer, sizeof(tinyBuffer), utc, "%Y", smallLength),
              ERR_COMM_BUFFER_OVERFLOW);
}

TEST(DateTimeErrorCodeApi, ValueReturningOverloads)
{
    constexpr TimestampMs timestamp = 0;  // 1970-01-01 00:00:00 UTC
    DateTimeSt utc{};
    ASSERT_EQ(utc_date_time(timestamp, utc), ERR_COMM_SUCCESS);

    // 值返回版：直接拿结果，不关心错误码
    EXPECT_EQ(format_time_string(utc), "1970-01-01 00:00:00");
    EXPECT_EQ(format_time_string(timestamp),
              format_time_string(timestamp, origin::date_time::DEFAULT_TIME_FMT, TimeZone::LOCAL));
    EXPECT_EQ(format_time_string(timestamp, "%Y", TimeZone::UTC), "1970");
    EXPECT_EQ(format_time_string(utc, "%Y/%m/%d"), "1970/01/01");

    char buffer[origin::date_time::MAX_TIME_STR_LEN] = {};
    EXPECT_EQ(format_time_buffer(buffer, sizeof(buffer), utc),
              std::string("1970-01-01 00:00:00").size());
    EXPECT_STREQ(buffer, "1970-01-01 00:00:00");

    char utcBuffer[origin::date_time::MAX_TIME_STR_LEN] = {};
    EXPECT_EQ(format_time_buffer(utcBuffer, sizeof(utcBuffer), timestamp, "%Y", TimeZone::UTC), 4u);
    EXPECT_STREQ(utcBuffer, "1970");

    // 缓冲区不足时值返回版返回 0（错误码版会返回 ERR_COMM_BUFFER_OVERFLOW）
    char tinyBuffer[2] = {};
    EXPECT_EQ(format_time_buffer(tinyBuffer, sizeof(tinyBuffer), utc), 0u);
}

TEST(UtilsErrorCodeApi, GetterValueReturningOverloads)
{
    // date_time：转换与名称查询
    constexpr TimestampMs epoch = 0;
    EXPECT_EQ(utc_date_time(epoch).year, 1970u);
    EXPECT_EQ(timestamp_to_date_time(epoch, TimeZone::UTC).year, 1970u);
    EXPECT_EQ(date_time_to_timestamp(local_date_time(epoch)), epoch);
    EXPECT_EQ(get_month_full_name(1), "January");
    EXPECT_EQ(get_month_abbr_name(1), "Jan");
    EXPECT_EQ(get_weekday_full_name(1), "Monday");
    EXPECT_EQ(get_weekday_abbr_name(1), "Mon");
    EXPECT_TRUE(get_month_full_name(13).empty());
    EXPECT_TRUE(get_weekday_full_name(9).empty());

    // filesystem：路径与文件查询
    EXPECT_FALSE(get_curr_working_dir().empty());
    EXPECT_FALSE(join_paths({"a", "b"}).empty());
    EXPECT_EQ(normalize_path("a/./b/../c"), normalize_path("a/c"));

    std::string dir;
    ASSERT_EQ(to_absolute_path("origin_ec_test_value_dir", get_curr_working_dir(), dir),
              ERR_COMM_SUCCESS);
    (void)delete_dir(dir, true);
    ASSERT_EQ(create_dir(dir, true), ERR_COMM_SUCCESS);
    std::string file;
    ASSERT_EQ(join_paths({dir, "value.txt"}, file), ERR_COMM_SUCCESS);
    ASSERT_EQ(create_file(file), ERR_COMM_SUCCESS);
    ASSERT_EQ(write_text_file(file, "hello", true), ERR_COMM_SUCCESS);

    EXPECT_EQ(get_entry_type(file), EntryType::FILE);
    EXPECT_EQ(get_entry_type(file + ".missing"), EntryType::NONEXISTENT);
    EXPECT_EQ(read_text_file(file), "hello");
    EXPECT_EQ(get_file_size(file), 5u);
    EXPECT_GT(get_file_modify_time(file), 0);
    const FileInfo info = get_file_info(file);
    EXPECT_EQ(info.type, EntryType::FILE);
    EXPECT_EQ(info.size, 5u);
    const std::vector<uint8_t> binary = read_binary_file(file);
    ASSERT_EQ(binary.size(), 5u);
    EXPECT_EQ(binary[0], static_cast<uint8_t>('h'));

    ASSERT_EQ(delete_dir(dir, true), ERR_COMM_SUCCESS);

    // process / strings / thread
    EXPECT_FALSE(origin::process::get_proc_path().empty());
    EXPECT_FALSE(origin::process::get_proc_directory().empty());
    EXPECT_FALSE(origin::process::get_proc_filename_stem().empty());
    EXPECT_EQ(FormatViaVaList("%d-%s", 7, "x"), "7-x");

    // 线程名是线程局部状态，测试后恢复原值，避免影响其他用例
    std::string originalName;
    ASSERT_EQ(origin::thread::get_curr_thread_name(originalName), ERR_COMM_SUCCESS);
    ASSERT_EQ(origin::thread::set_curr_thread_name("value-overload"), ERR_COMM_SUCCESS);
    EXPECT_EQ(origin::thread::get_curr_thread_name(), "value-overload");
    ASSERT_EQ(origin::thread::set_curr_thread_name(originalName.c_str()), ERR_COMM_SUCCESS);
}

}  // namespace
