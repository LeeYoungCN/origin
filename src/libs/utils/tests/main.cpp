#include "gtest/gtest.h"
#include "origin/common/debug/debug_logger.h"
#include "origin/common/version.h"

int main(int argc, char* argv[])
{
    origin_set_debug_logger_level(ORG_DBG_LVL_INFO);
    ORIGIN_DEBUG_INFO("Running main() from {}", __FILE__);
    ORIGIN_DEBUG_INFO("Origin {}", origin::version());
    if (!origin::headers_match_binary()) {
        ORIGIN_DEBUG_ERR("Header/binary version mismatch: headers={} binary={}",
                         origin::compiled_version().string,
                         origin::version());
    }
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
