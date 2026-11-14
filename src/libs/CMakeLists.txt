set_property(GLOBAL PROPERTY ORIGIN_LIB_LIST "")
set(THREADS_PREFER_PTHREAD_FLAG TRUE)   # 必须在 find_package 之前设置
find_package(Threads REQUIRED)

include(${CMAKE_SOURCE_DIR}/cmake/version_resource.cmake)

function(origin_add_component lib)
    set(origin_output_name "${PROJECT_NAME}${lib}")
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

    set_target_properties(${lib} PROPERTIES
        OUTPUT_NAME   "${origin_output_name}"
        DEBUG_POSTFIX "d"
        VERSION       ${PROJECT_VERSION}
        SOVERSION     ${PROJECT_VERSION_MAJOR}
    )

    # Windows 专属：嵌入版本资源，资源管理器里能看到文件/产品版本
    origin_add_version_resource(${lib} "${origin_output_name}" "${lib} library of ${PROJECT_NAME}")

    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT development
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
endfunction()


# 显式列出组件目录，顺序按依赖排列（common → container → utils → logging）。
# 新增组件时在这里加一行，不要在 libs 下做目录扫描。
add_subdirectory(common)
add_subdirectory(container)
add_subdirectory(utils)
add_subdirectory(logging)

get_property(LIB_LIST GLOBAL PROPERTY ORIGIN_LIB_LIST)
message(STATUS "LIB_LIST: ${LIB_LIST}")

include(${CMAKE_SOURCE_DIR}/cmake/package/main_config.cmake)
