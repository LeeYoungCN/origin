#ifndef COMMON_COMPILER_WARNINGS_H
#define COMMON_COMPILER_WARNINGS_H

#include "origin/common/macros/compiler.h"

// 警告级别由构建系统（target 级编译选项）控制；头文件不改变包含方的全局警告状态。
// 需要临时屏蔽警告时，在局部范围内成对使用：
//
//   ORIGIN_WARNINGS_PUSH
//   ...可能触发警告的声明...
//   ORIGIN_WARNINGS_POP
#if COMPILER_MSVC
#define ORIGIN_WARNINGS_PUSH _Pragma("warning(push, 4)")
#define ORIGIN_WARNINGS_POP  _Pragma("warning(pop)")
#elif COMPILER_GCC || COMPILER_CLANG
#define ORIGIN_WARNINGS_PUSH _Pragma("GCC diagnostic push")
#define ORIGIN_WARNINGS_POP  _Pragma("GCC diagnostic pop")
#else
#define ORIGIN_WARNINGS_PUSH
#define ORIGIN_WARNINGS_POP
#endif

// 屏蔽"未使用变量"警告（用于必须定义但暂时不用的变量）
#define UNUSED(var) (void)(var)

#endif  // COMMON_COMPILER_WARNINGS_H
