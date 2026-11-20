set_property(GLOBAL PROPERTY ORIGIN_LIB_LIST "")
set(THREADS_PREFER_PTHREAD_FLAG TRUE)   # 必须在 find_package 之前设置
find_package(Threads REQUIRED)

include(${PROJECT_SOURCE_DIR}/cmake/version_resource.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/origin_component.cmake)


# 显式列出组件目录，顺序按依赖排列（common → container → utils → logging）。
# 新增组件时在这里加一行，不要在 libs 下做目录扫描。
add_subdirectory(common)
add_subdirectory(container)
add_subdirectory(utils)
add_subdirectory(logging)

get_property(LIB_LIST GLOBAL PROPERTY ORIGIN_LIB_LIST)
message(STATUS "LIB_LIST: ${LIB_LIST}")

include(${PROJECT_SOURCE_DIR}/cmake/package/install_export.cmake)
