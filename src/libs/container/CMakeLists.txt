set(LIB_NAME Container)

set_property(GLOBAL APPEND PROPERTY ORIGIN_LIB_LIST ${LIB_NAME})

add_library(${LIB_NAME} INTERFACE)

# 构建树也提供 Origin::Container，和安装树导出的目标名保持一致
add_library(${PROJECT_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

target_include_directories(${LIB_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  # 编译时路径
        $<INSTALL_INTERFACE:include>                            # 安装时路径
)

# 头文件库同样把 C++20 要求传给消费者
target_compile_features(${LIB_NAME} INTERFACE cxx_std_20)

if (ORIGIN_INSTALL)
    install(DIRECTORY include/
            DESTINATION include  # 安装路径：${CMAKE_INSTALL_PREFIX}/include
            FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
endif()

if (ORIGIN_BUILD_TESTS)
    add_subdirectory(tests)
endif()
