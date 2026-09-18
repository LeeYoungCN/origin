set(TEST_RUNTIME test_${LIB_NAME})
set(TEST_OBJECT  test_${LIB_NAME}_obj)

file(GLOB_RECURSE SRC_LIST "src/*.cpp")

add_library(${TEST_OBJECT} OBJECT ${SRC_LIST})
add_executable(${TEST_RUNTIME} main.cpp $<TARGET_OBJECTS:${TEST_OBJECT}>)

target_link_libraries(${TEST_OBJECT} PUBLIC ${LIB_NAME} GTest::gtest)
target_link_libraries(${TEST_RUNTIME} PUBLIC ${LIB_NAME} GTest::gtest)

if (MINGW)
    target_link_libraries(${TEST_OBJECT} PUBLIC pthread)
    target_link_libraries(${TEST_RUNTIME} PUBLIC pthread)
endif()

list(APPEND TEST_OBJ_LIST $<TARGET_OBJECTS:${TEST_OBJECT}>)
set(TEST_OBJ_LIST "${TEST_OBJ_LIST}" CACHE STRING "" FORCE)

# include(GoogleTest)
# gtest_discover_tests(${TEST_RUNTIME})


add_test(NAME ${TEST_RUNTIME} COMMAND ${TEST_RUNTIME})
set_tests_properties(${TEST_RUNTIME} PROPERTIES TIMEOUT 60)
