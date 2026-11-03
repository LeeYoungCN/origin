# 设置所有依赖库的根目录
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/external CACHE PATH "下载依赖库的根目录")
# 第三方库安装路径
set(THIRD_PARTY_INSTALL_DIR ${CMAKE_SOURCE_DIR}/out/third_party CACHE PATH "第三方库安装路径")

# 使用FetchContent模块
include(FetchContent)

# 声明并下载Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  # 1.12.1(2022) 用 /W4 /WX 编译自身，clang-cl 16+ 的 /W4 会打开
  # -Wcharacter-conversion，命中 gtest-printers.h 里 char16_t/char8_t ->
  # char32_t 的隐式转换，再被 /WX 升级成 error。v1.18.0 起 gtest 不再对
  # 自身用 /WX。
  GIT_TAG        v1.18.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# v1.18.0 的 gtest-printers.h 仍保留 ImplicitCast_<char32_t>(char16_t/char8_t)，
# clang-cl 的 /W4 会报 -Wcharacter-conversion（现在只是 warning，不再 error）。
# 只对 gtest 自身的目标关掉这一条，不影响项目代码的告警策略。
if (MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  foreach(tgt gtest gtest_main gmock gmock_main)
    if (TARGET ${tgt})
      target_compile_options(${tgt} PRIVATE -Wno-character-conversion)
    endif()
  endforeach()
endif()
