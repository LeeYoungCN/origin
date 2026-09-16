if (BUILD_TESTS)
    set_property(GLOBAL PROPERTY TEST_TIMEOUT 60)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_THIRD_PARTY_PATH}/googletest")

    find_package(GTest QUIET)

    if (NOT GTest_FOUND)
        message(STATUS "GTest not found, fetching with FetchContent")
        include(${CMAKE_SOURCE_DIR}/cmake/fetch_content.cmake)
    endif()
endif()

set(LIB_LIST "" CACHE STRING "Origin lib list" FORCE)
set(TEST_OBJ_LIST "" CACHE STRING "Test objects list" FORCE)
add_subdirectory(libs)

message(STATUS "LIB_LIST: ${LIB_LIST}")

if (BUILD_TEST_ORIGIN)
    message(STATUS "TEST_OBJ_LIST: ${TEST_OBJ_LIST}")
    add_subdirectory(tests)
endif()
