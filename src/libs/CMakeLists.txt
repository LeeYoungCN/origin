set_property(GLOBAL PROPERTY ORIGIN_LIB_LIST "")
set(THREADS_PREFER_PTHREAD_FLAG TRUE)   # 必须在 find_package 之前设置
find_package(Threads REQUIRED)

function(origin_add_component lib)
    # 只负责配置组件；源文件由各组件目录自己 glob 后交给 add_library。
    set_property(GLOBAL APPEND PROPERTY ORIGIN_LIB_LIST ${lib})

    target_include_directories(${lib}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  # 编译时路径
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>        # 生成的 version.h
            $<INSTALL_INTERFACE:include>                            # 安装时路径
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src                        # 内部头文件路径
    )

    target_link_libraries(${lib} PUBLIC Threads::Threads)

    set_target_properties(${lib} PROPERTIES
        OUTPUT_NAME   "${PROJECT_NAME}${lib}"
        DEBUG_POSTFIX "d"
        VERSION       ${PROJECT_VERSION}
        SOVERSION     ${PROJECT_VERSION_MAJOR}
    )

    install(DIRECTORY include/
        DESTINATION include  # 安装路径：${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
endfunction()


file(GLOB LIBS_DIR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(DIR ${LIBS_DIR})
    if (IS_DIRECTORY ${DIR})
        add_subdirectory(${DIR})
    endif()
endforeach()

get_property(LIB_LIST GLOBAL PROPERTY ORIGIN_LIB_LIST)
message(STATUS "LIB_LIST: ${LIB_LIST}")

include(${CMAKE_SOURCE_DIR}/cmake/package/main_config.cmake)
