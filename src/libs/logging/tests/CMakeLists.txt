set(TEST_LIB_NAME ${LIB_NAME}_test)

list(APPEND TEST_LIB_LIST ${TEST_LIB_NAME})
set(TEST_LIB_LIST "${TEST_LIB_LIST}" CACHE STRING "" FORCE)

add_library(${TEST_LIB_NAME} STATIC)

file(GLOB_RECURSE SRC_LIST "src/*.cpp")
target_sources(${TEST_LIB_NAME} PRIVATE ${SRC_LIST})

target_include_directories(${TEST_LIB_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>  # 编译时路径
        ${CMAKE_CURRENT_SOURCE_DIR}/../src                         # 内部头文件路径
)

target_link_libraries(${TEST_LIB_NAME} PUBLIC ${LIB_NAME} GTest::gtest)

if (MINGW)
    target_link_libraries(${TEST_LIB_NAME} PUBLIC pthread)
endif()

set_target_properties(${TEST_LIB_NAME} PROPERTIES
    PREFIX "lib"          # 给输出文件名加前缀
    IMPORT_PREFIX "lib"   # 给导入库(.lib)加前缀
)

set_target_properties(${TEST_LIB_NAME} PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 自动导出所有符号
)
