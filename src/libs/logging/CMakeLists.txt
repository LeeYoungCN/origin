set(LIB_NAME logging)
list(APPEND LIB_LIST ${LIB_NAME})
set(LIB_LIST "${LIB_LIST}" CACHE STRING "" FORCE)

add_library(${LIB_NAME} SHARED)
target_compile_definitions(${LIB_NAME} PRIVATE LOGGING_BUILD_SHARED)

file(GLOB_RECURSE SRC_LIST src "*.cpp" "*.c")
target_sources(${LIB_NAME} PRIVATE ${SRC_LIST})

target_include_directories(${LIB_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  # 编译时路径
        $<INSTALL_INTERFACE:include>                            # 安装时路径
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src                        # 内部头文件路径
)

if (MINGW)
    target_link_libraries(${LIB_NAME} PRIVATE pthread)
endif()

set_target_properties(${LIB_NAME} PROPERTIES
    PREFIX "lib"          # 给输出文件名加前缀
    IMPORT_PREFIX "lib"   # 给导入库(.lib)加前缀
)

target_link_libraries(${LIB_NAME} PRIVATE common utils)

set_target_properties(${LIB_NAME} PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 自动导出所有符号
)

install(DIRECTORY include/
        DESTINATION include  # 安装路径：${CMAKE_INSTALL_PREFIX}/include
        COMPONENT ${LIB_NAME}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

install(TARGETS ${LIB_NAME} COMPONENT ${LIB_NAME})
