#include "common/debug/debug_logger.h"
#include "gtest/gtest.h"

int main(int argc, char *argv[])
{
    origin_set_debug_logger_level(ORG_DBG_LVL_INFO);
    ORIGIN_DEBUG_INFO("Running main() from {}", __FILE__);

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
