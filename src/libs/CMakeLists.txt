set(LIB_LIST "" CACHE STRING "Origin lib list" FORCE)

file(GLOB LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(DIR ${LIBS_DIR})
    if (IS_DIRECTORY ${DIR})
        add_subdirectory(${DIR})
    endif()
endforeach()

message(STATUS "LIB_LIST: ${LIB_LIST}")

include(${CMAKE_SOURCE_DIR}/cmake/package/main_config.cmake)
