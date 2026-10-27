#include "detail/common.hpp"
#include "gtest/gtest.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/utils/filesystem_utils.h"

int main(int argc, char *argv[])
{
    auto dir = logging_test::get_log_dir();

    origin_set_debug_logger_level(ORG_DBG_LVL_INFO);
    ORIGIN_DEBUG_INFO("Running main() from \"{}\".", __FILE__);
    ORIGIN_DEBUG_INFO("Log file directory: \"{}\".", dir);

    testing::InitGoogleTest(&argc, argv);
    auto rst = RUN_ALL_TESTS();
    ORIGIN_DEBUG_INFO("Run all tests finish. Result: {}.", rst);
    origin::filesystem::delete_dir(dir);

    return rst;
}
