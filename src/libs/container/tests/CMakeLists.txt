# 显式源文件清单：新增/删除测试文件时同步改这里。
set(SRC_LIST
    src/test_blocking_queue.cpp
    src/test_concurrent_blocking_queue_mt.cpp
    src/test_concurrent_blocking_queue_st.cpp
)

origin_add_component_tests(${LIB_NAME} LIBRARIES Common Utils)
