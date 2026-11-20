# 组件构建脚手架：库目标与测试目标的统一创建入口。
#
# 这里只放 Origin 自己的构建函数（不安装、不导出）：
#   origin_add_component(<lib>)         创建组件库的公共配置
#   origin_add_component_tests(<lib>)   创建组件测试目标与 CTest 用例

# 配置组件的库目标：include 路径、Threads、警告、产物名、版本资源、安装规则。
# 源文件由各组件目录自己 add_library，再调用本函数。
function(origin_add_component lib)
    set(origin_output_name "${PROJECT_NAME}${lib}")

    # 构建树与安装树使用同一套目标名（安装树导出 Origin::xxx）
    add_library(${PROJECT_NAME}::${lib} ALIAS ${lib})

    # 只负责配置组件；源文件由各组件目录自己 glob 后交给 add_library。
    set_property(GLOBAL APPEND PROPERTY ORIGIN_LIB_LIST ${lib})

    target_include_directories(${lib}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  # 编译时路径
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>        # 生成的 version.h
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>        # 安装时路径
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src                        # 内部头文件路径
    )

    target_link_libraries(${lib} PUBLIC Threads::Threads)

    # 警告只作用于 Origin 自己的库，第三方（FetchContent）不受影响
    if (ORIGIN_WARNING_OPTIONS)
        target_compile_options(${lib} PRIVATE ${ORIGIN_WARNING_OPTIONS})
    endif()

    set_target_properties(${lib} PROPERTIES
        OUTPUT_NAME   "${origin_output_name}"
        DEBUG_POSTFIX "d"
        VERSION       ${PROJECT_VERSION}
        SOVERSION     ${PROJECT_VERSION_MAJOR}
    )

    # MinGW 默认给 DLL 加 lib 前缀（libOriginLoggingd.dll），与 MSVC 的命名不一致；
    # Windows 上统一去掉，保证两种工具链的产物名一致
    if (WIN32)
        set_target_properties(${lib} PROPERTIES PREFIX "")
    endif()

    # Windows 专属：嵌入版本资源，资源管理器里能看到文件/产品版本
    origin_add_version_resource(${lib} "${origin_output_name}" "${lib} library of ${PROJECT_NAME}")

    # 调试符号：MSVC / clang-cl 生成 PDB，随 Debug 包一起安装，方便单步进库代码
    if(WIN32 AND NOT MINGW)
        install(FILES $<TARGET_PDB_FILE:${lib}> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
    endif()

    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT development
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
endfunction()

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

    # 默认随机化测试顺序，尽早暴露测试间的状态依赖
    add_test(NAME ${test_runtime} COMMAND ${test_runtime} --gtest_shuffle)
    set_tests_properties(${test_runtime} PROPERTIES TIMEOUT 60)
endfunction()
