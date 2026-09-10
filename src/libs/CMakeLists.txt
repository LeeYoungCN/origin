file(GLOB LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(DIR ${LIBS_DIR})
    if (IS_DIRECTORY ${DIR})
        add_subdirectory(${DIR})
        # list(APPEND LIB_INC_LIST "${DIR}/include/")
    endif()
endforeach()

# set(LIB_INC_LIST "${LIB_INC_LIST}" CACHE STRING "" FORCE)

# install(DIRECTORY ${LIB_INC_LIST}
#         DESTINATION include  # 安装路径：${CMAKE_INSTALL_PREFIX}/include
#         COMPONENT runtime
#         FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

# install(TARGETS ${LIB_LIST} COMPONENT runtime)

