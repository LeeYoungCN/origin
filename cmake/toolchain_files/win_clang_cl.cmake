# 注意：VS 18 的 MSVC STL 14.52 要求 Clang 22+，clang-cl 过旧会触发 STL1000；
# 本地使用 win_clang_cl_* 预设时请确保 clang-cl >= 22（CI 会在构建前安装 LLVM 22）。
set(CMAKE_C_COMPILER "clang-cl.exe")
set(CMAKE_CXX_COMPILER "clang-cl.exe")
