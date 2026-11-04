set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
# 安装后的 RPATH（@loader_path/...）在顶层 CMakeLists.txt 统一设置。
