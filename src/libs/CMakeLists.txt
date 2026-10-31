set(LIB_LIST "" CACHE STRING "Origin lib list" FORCE)

function(set_lib_public_properties lib)
    list(APPEND LIB_LIST ${LIB_NAME})
    set(LIB_LIST "${LIB_LIST}" CACHE STRING "" FORCE)

    file(GLOB_RECURSE SRC_LIST "src/*.cpp" "src/*.c")
    target_sources(${LIB_NAME} PRIVATE ${SRC_LIST})

    target_include_directories(${lib}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  # 编译时路径
            $<INSTALL_INTERFACE:include>                            # 安装时路径
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src                        # 内部头文件路径
    )

    if (MINGW)
        target_link_libraries(${lib} PUBLIC pthread)
    endif()

    set_target_properties(${lib} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}${LIB_NAME}"
        DEBUG_POSTFIX "d"
    )

    install(DIRECTORY include/
        DESTINATION include  # 安装路径：${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
endfunction()


file(GLOB LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(DIR ${LIBS_DIR})
    if (IS_DIRECTORY ${DIR})
        add_subdirectory(${DIR})
    endif()
endforeach()

message(STATUS "LIB_LIST: ${LIB_LIST}")

include(${CMAKE_SOURCE_DIR}/cmake/package/main_config.cmake)
