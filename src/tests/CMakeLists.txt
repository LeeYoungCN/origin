set(TEST_RUNTIME test_origin)

get_property(LIB_LIST GLOBAL PROPERTY ORIGIN_LIB_LIST)
get_property(TEST_OBJ_LIST GLOBAL PROPERTY ORIGIN_TEST_OBJ_LIST)

add_executable(${TEST_RUNTIME}
    main.cpp
    test_version.cpp
    ${TEST_OBJ_LIST})

target_link_libraries(${TEST_RUNTIME} GTest::gtest ${LIB_LIST})

target_compile_features(${TEST_RUNTIME} PRIVATE cxx_std_20)
origin_apply_compile_options(${TEST_RUNTIME})
origin_apply_link_options(${TEST_RUNTIME})
