# set(TEST_RUNTIME_NAME test_${LIB_NAME})

# add_executable(${TEST_RUNTIME_NAME})

# file(GLOB_RECURSE SRC_LIST "*.cpp")
# target_sources(${TEST_RUNTIME_NAME} PRIVATE ${SRC_LIST})

# target_link_libraries(${TEST_RUNTIME_NAME} PUBLIC ${LIB_NAME} common utils GTest::gtest)

# if (MINGW)
#     target_link_libraries(${TEST_RUNTIME_NAME} PUBLIC pthread)
# endif()

# include(GoogleTest)
# gtest_discover_tests(${TEST_RUNTIME_NAME})


set(TEST_RUNTIME ${LIB_NAME}_test)
set(TEST_OBJECT  ${LIB_NAME}_test_obj)

file(GLOB_RECURSE SRC_LIST "src/*.cpp")

add_library(${TEST_OBJECT} OBJECT ${SRC_LIST})
add_executable(${TEST_RUNTIME} main.cpp $<TARGET_OBJECTS:${TEST_OBJECT}>)

target_link_libraries(${TEST_OBJECT} PUBLIC ${LIB_NAME} common utils GTest::gtest)
target_link_libraries(${TEST_RUNTIME} PUBLIC ${LIB_NAME} common utils GTest::gtest)

if (MINGW)
    target_link_libraries(${TEST_OBJECT} PUBLIC pthread)
    target_link_libraries(${TEST_RUNTIME} PUBLIC pthread)
endif()

list(APPEND TEST_OBJ_LIST $<TARGET_OBJECTS:${TEST_OBJECT}>)
set(TEST_OBJ_LIST "${TEST_OBJ_LIST}" CACHE STRING "" FORCE)

include(GoogleTest)
gtest_discover_tests(${TEST_RUNTIME})
