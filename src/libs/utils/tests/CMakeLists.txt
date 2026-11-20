# 显式源文件清单：新增/删除测试文件时同步改这里。
set(SRC_LIST
    # date_time
    src/test_date_time_utils/test_date_time_utils.cpp
    src/test_date_time_utils/test_date_time_utils_common.cpp
    src/test_date_time_utils/test_date_time_utils_string_date_time.cpp
    src/test_date_time_utils/test_date_time_utils_string_timestamp.cpp
    # filesystem
    src/test_filesystem_utils/test_filesystem_utils_dir.cpp
    src/test_filesystem_utils/test_filesystem_utils_file.cpp
    src/test_filesystem_utils/test_filesystem_utils_path.cpp
    src/test_filesystem_utils/test_file_writer.cpp
    # thread
    src/test_thread_utils/test_thread_utils.cpp
    # error-code APIs
    src/test_error_code_api.cpp
)

origin_add_component_tests(${LIB_NAME})
