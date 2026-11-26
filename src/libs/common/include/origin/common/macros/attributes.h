#ifndef COMMON_COMPILER_ATTRIBUTES_H
#define COMMON_COMPILER_ATTRIBUTES_H

// 强制内联（比inline更激进）
#ifdef _MSC_VER
#define ATTR_FORCE_INLINE __forceinline
#else
#define ATTR_FORCE_INLINE __attribute__((always_inline))
#endif

// 禁止内联（用于调试或性能敏感场景）
#ifdef _MSC_VER
#define ATTR_NOINLINE __declspec(noinline)
#else
#define ATTR_NOINLINE __attribute__((noinline))
#endif

// 标记废弃函数（调用时编译器会警告）
#ifdef _MSC_VER
#define ATTR_DEPRECATED(message) __declspec(deprecated(message))
#else
#define ATTR_DEPRECATED(message) __attribute__((deprecated(message)))
#endif

// 对齐控制（指定变量/类型的内存对齐字节数）
#ifdef _MSC_VER
#define ATTR_ALIGNED(alignment) __declspec(align(alignment))
#else
#define ATTR_ALIGNED(alignment) __attribute__((aligned(alignment)))
#endif

#endif  // COMMON_COMPILER_ATTRIBUTES_H
