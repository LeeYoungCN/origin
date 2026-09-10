set(TEST_RUNTIME origin_test)

add_executable(${TEST_RUNTIME}
    main.cpp
    ${TEST_OBJ_LIST})

target_link_libraries(${TEST_RUNTIME} GTest::gtest ${LIB_LIST})
