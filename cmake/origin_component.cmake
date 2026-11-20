# 组件构建脚手架：库目标与测试目标的统一创建入口。
#
# 这里只放 Origin 自己的构建函数（不安装、不导出）：
#   origin_apply_compile_options(<target>)  目标级编译选项（MSVC/警告/ASAN/测试宏）
#   origin_apply_link_options(<target>)    目标级链接选项（ASAN）
#   origin_add_component(<lib>)             创建组件库的公共配置
#   origin_add_component_tests(<lib>)       创建组件测试目标与 CTest 用例

# 编译选项统一走 target 级，避免全局 add_compile_options 污染 GTest、Qt 等第三方目标。
function(origin_apply_compile_options target)
    if (MSVC)
        target_compile_options(${target} PRIVATE
            /utf-8
            $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>)
        if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # clang-cl 不支持这两个 MSVC 专有选项
            target_compile_options(${target} PRIVATE
                $<$<COMPILE_LANGUAGE:CXX>:/permissive->
                $<$<COMPILE_LANGUAGE:CXX>:/Zc:preprocessor>)
        endif()
    endif()

    if (ORIGIN_WARNING_OPTIONS)
        target_compile_options(${target} PRIVATE ${ORIGIN_WARNING_OPTIONS})
    endif()

    if (ORIGIN_ENABLE_ASAN)
        if (MSVC)
            target_compile_options(${target} PRIVATE /fsanitize=address /Zi)
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        endif()
    endif()

    if (ORIGIN_BUILD_TESTS)
        target_compile_definitions(${target} PRIVATE ORIGIN_ENABLE_TEST)
    endif()
endfunction()

function(origin_apply_link_options target)
    if (ORIGIN_ENABLE_ASAN)
        if (MSVC)
            target_link_options(${target} PRIVATE /fsanitize=address /INCREMENTAL:NO)
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_link_options(${target} PRIVATE -fsanitize=address)
        endif()
    endif()
endfunction()

# 配置组件的库目标：include 路径、Threads、C++ 标准、编译/链接选项、产物名、版本资源、安装规则。
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

    # 导出 C++20 要求：安装后的消费者也能自动获得 -std=c++20
    target_compile_features(${lib} PUBLIC cxx_std_20)

    set_target_properties(${lib} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        C_STANDARD 23
        C_STANDARD_REQUIRED ON
        OUTPUT_NAME   "${origin_output_name}"
        DEBUG_POSTFIX "d"
        VERSION       ${PROJECT_VERSION}
        SOVERSION     ${PROJECT_VERSION_MAJOR}
    )

    origin_apply_compile_options(${lib})
    origin_apply_link_options(${lib})

    # MinGW 默认给 DLL 加 lib 前缀（libOriginLoggingd.dll），与 MSVC 的命名不一致；
    # Windows 上统一去掉，保证两种工具链的产物名一致
    if (WIN32)
        set_target_properties(${lib} PROPERTIES PREFIX "")
    endif()

    # Windows 专属：嵌入版本资源，资源管理器里能看到文件/产品版本
    origin_add_version_resource(${lib} "${origin_output_name}" "${lib} library of ${PROJECT_NAME}")

    # 调试符号：MSVC / clang-cl 生成 PDB，随 Debug 包一起安装，方便单步进库代码
    if (ORIGIN_INSTALL)
        if(WIN32 AND NOT MINGW)
            install(FILES $<TARGET_PDB_FILE:${lib}> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
        endif()

        install(DIRECTORY include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            COMPONENT development
            FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
    endif()
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

    target_compile_features(${test_object} PUBLIC cxx_std_20)
    target_compile_features(${test_runtime} PRIVATE cxx_std_20)

    origin_apply_compile_options(${test_object})
    origin_apply_compile_options(${test_runtime})
    origin_apply_link_options(${test_runtime})

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
