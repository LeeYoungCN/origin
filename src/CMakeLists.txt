if (ORIGIN_BUILD_TESTS)
    set_property(GLOBAL PROPERTY TEST_TIMEOUT 60)

    # 优先用 third_lib 的接入脚本：它负责找 googletest 并建好 GTest::gtest / GTest::gtest_main
    # （先 find_package(GTest CONFIG)，找不到再手工建 imported target）。
    if (COMMAND third_lib_add_package)
        third_lib_add_package(googletest)
    else()
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_THIRD_PARTY_PATH}/googletest")
        find_package(GTest QUIET)
    endif()

    # 判 TARGET 而不是 GTest_FOUND：third_lib 在 find_package 失败时会自己建 GTest::gtest
    if (NOT TARGET GTest::gtest)
        message(STATUS "GTest not found, fetching with FetchContent")
        include(${PROJECT_SOURCE_DIR}/cmake/fetch_gtest.cmake)
    endif()
endif()

set_property(GLOBAL PROPERTY ORIGIN_TEST_OBJ_LIST "")
add_subdirectory(libs)

if (ORIGIN_BUILD_TESTS)
    get_property(TEST_OBJ_LIST GLOBAL PROPERTY ORIGIN_TEST_OBJ_LIST)
    message(STATUS "TEST_OBJ_LIST: ${TEST_OBJ_LIST}")
    add_subdirectory(tests)
endif()

