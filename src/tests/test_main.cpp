#include "common/debug/debug_logger.h"
#include "common/debug/working_env.h"
#include "gtest/gtest.h"

int main(int argc, char *argv[])
{
    origin_set_debug_logger_level(ORG_DBG_LVL_DEBUG);
    DEBUG_LOGGER_INFO("Running main() from {}", __FILE__);

    origin_show_working_env();

    testing::InitGoogleTest(&argc, argv);
    auto rst = RUN_ALL_TESTS();

    DEBUG_LOGGER_INFO("Run all tests finish. Result: {}.", rst);

    return rst;
}
