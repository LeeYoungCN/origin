#include <cstdint>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include <origin/common/common_error_code.h>
#include <origin/date_time/date_time.h>
#include <origin/filesystem/filesystem.h>
#include <origin/utils/utils_error_code.h>

namespace {

using namespace origin::date_time;
using namespace origin::filesystem;

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

    char small[2] = {};
    size_t smallLength = 0;
    EXPECT_EQ(format_time_buffer(small, sizeof(small), utc, "%Y", smallLength),
              ERR_COMM_BUFFER_OVERFLOW);
}

}  // namespace
