if (BUILD_TESTS)
    set_property(GLOBAL PROPERTY TEST_TIMEOUT 60)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_THIRD_PARTY_PATH}/googletest")

    find_package(GTest QUIET)

    if (NOT GTest_FOUND)
        message(STATUS "GTest not found, fetching with FetchContent")
        include(${CMAKE_SOURCE_DIR}/cmake/fetch_content.cmake)
    endif()
endif()

set_property(GLOBAL PROPERTY ORIGIN_TEST_OBJ_LIST "")
add_subdirectory(libs)

if (BUILD_TESTS)
    get_property(TEST_OBJ_LIST GLOBAL PROPERTY ORIGIN_TEST_OBJ_LIST)
    message(STATUS "TEST_OBJ_LIST: ${TEST_OBJ_LIST}")
    add_subdirectory(tests)
endif()

