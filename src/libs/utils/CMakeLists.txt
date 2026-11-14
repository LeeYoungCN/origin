set(LIB_NAME Utils)

# 显式源文件清单：新增/删除源文件时同步改这里。
set(SRC_LIST
    # date_time
    src/date_time/date_time_utils.cpp
    src/date_time/date_time_utils_string.cpp
    # filesystem
    src/filesystem/filesystem_utils.cpp
    src/filesystem/filesystem_utils_directory.cpp
    src/filesystem/filesystem_utils_file.cpp
    src/filesystem/filesystem_utils_path.cpp
    src/filesystem/file_writer.cpp
    src/filesystem/internal/common.cpp
    src/filesystem/internal/file_writer_impl.cpp
    # process / string / thread
    src/process_utils.cpp
    src/string_utils.cpp
    src/thread_utils.cpp
)
add_library(${LIB_NAME} SHARED ${SRC_LIST})
target_compile_definitions(${LIB_NAME} PRIVATE UTILS_BUILD_SHARED)

origin_add_component(${LIB_NAME})

set_target_properties(${LIB_NAME} PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 自动导出所有符号
)

target_link_libraries(${LIB_NAME} PUBLIC Common)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
