# 为组件创建统一的测试目标：OBJECT 库 + 可执行文件 + CTest 用例。
#
# 用法（在各组件的 tests/CMakeLists.txt 中）：
#   set(SRC_LIST ...)                 # 测试源文件
#   origin_add_component_tests(${LIB_NAME})
#
# 可选参数：
#   SOURCES <src...>       测试源文件；省略时使用调用方作用域的 SRC_LIST
#   LIBRARIES <lib...>     额外链接的库（组件自身、GTest、Threads 会自动链接）
#   INCLUDE_DIRS <dir...>  额外的 include 目录（PUBLIC 到测试目标）
#   MAIN <file>            测试入口，默认 main.cpp
#
# 生成：
#   test_<lib>_obj（OBJECT）、test_<lib>（可执行文件）和 CTest 用例 test_<lib>
function(origin_add_component_tests lib)
    cmake_parse_arguments(ARG "" "MAIN" "SOURCES;LIBRARIES;INCLUDE_DIRS" ${ARGN})

    if (ARG_SOURCES)
        set(test_sources ${ARG_SOURCES})
    else()
        set(test_sources ${SRC_LIST})
    endif()
    if (NOT test_sources)
        message(FATAL_ERROR "origin_add_component_tests(${lib}): 没有测试源文件")
    endif()

    if (ARG_MAIN)
        set(test_main ${ARG_MAIN})
    else()
        set(test_main main.cpp)
    endif()

    set(test_runtime "test_${lib}")
    set(test_object "test_${lib}_obj")

    add_library(${test_object} OBJECT ${test_sources})
    add_executable(${test_runtime} ${test_main} $<TARGET_OBJECTS:${test_object}>)

    set(test_link_libs ${lib} ${ARG_LIBRARIES} GTest::gtest)
    target_link_libraries(${test_object} PUBLIC ${test_link_libs})
    target_link_libraries(${test_runtime} PUBLIC ${test_link_libs})
    target_link_libraries(${test_object} PRIVATE Threads::Threads)
    target_link_libraries(${test_runtime} PRIVATE Threads::Threads)

    if (ARG_INCLUDE_DIRS)
        target_include_directories(${test_object} PUBLIC ${ARG_INCLUDE_DIRS})
        target_include_directories(${test_runtime} PUBLIC ${ARG_INCLUDE_DIRS})
    endif()

    # 聚合测试（test_origin）复用各组件的 OBJECT 列表
    set_property(GLOBAL APPEND PROPERTY ORIGIN_TEST_OBJ_LIST $<TARGET_OBJECTS:${test_object}>)

    if (ORIGIN_TEST_SHUFFLE)
        add_test(NAME ${test_runtime} COMMAND ${test_runtime} --gtest_shuffle)
    else()
        add_test(NAME ${test_runtime} COMMAND ${test_runtime})
    endif()
    set_tests_properties(${test_runtime} PROPERTIES TIMEOUT 60)
endfunction()
