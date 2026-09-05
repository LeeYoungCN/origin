if (BUILD_TESTS)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_THIRD_PARTY_PATH}/googletest")

    find_package(GTest QUIET)

    if (NOT GTest_FOUND)
        message(STATUS "GTest not found, fetching with FetchContent")
        include(${CMAKE_SOURCE_DIR}/cmake/fetch_content.cmake)
    endif()
    set(TEST_LIB_LIST "" CACHE STRING "")
endif()

set(LIB_LIST "" CACHE STRING "")
add_subdirectory(libs)

message(STATUS "LIB_LIST: ${LIB_LIST}")

if (BUILD_TESTS)
    message(STATUS "TEST_LIB_LIST: ${TEST_LIB_LIST}")
    add_subdirectory(tests)
endif()
