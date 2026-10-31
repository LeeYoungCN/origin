set(LIB_NAME Container)
list(APPEND LIB_LIST ${LIB_NAME})
set(LIB_LIST "${LIB_LIST}" CACHE STRING "" FORCE)

add_library(${LIB_NAME} INTERFACE)

target_include_directories(${LIB_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  # 编译时路径
        $<INSTALL_INTERFACE:include>                            # 安装时路径
)

set_target_properties(${lib} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}${LIB_NAME}"
    DEBUG_POSTFIX "d"
)

install(DIRECTORY include/
        DESTINATION include  # 安装路径：${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
