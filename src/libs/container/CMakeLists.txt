set(LIB_NAME container)
list(APPEND LIB_LIST ${LIB_NAME})
set(LIB_LIST "${LIB_LIST}" CACHE STRING "" FORCE)

add_library(${LIB_NAME} INTERFACE)

target_include_directories(${LIB_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  # 编译时路径
        $<INSTALL_INTERFACE:include>                            # 安装时路径
)

install(DIRECTORY include/
        DESTINATION include  # 安装路径：${CMAKE_INSTALL_PREFIX}/include
        COMPONENT ${LIB_NAME}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
