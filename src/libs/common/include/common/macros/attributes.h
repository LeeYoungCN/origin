#ifndef COMMON_COMPILER_ATTRIBUTES_H
#define COMMON_COMPILER_ATTRIBUTES_H

// 强制编译器保留符号（避免被优化）
#ifdef _MSC_VER
#define ATTR_USED __declspec(used)
#else
#define ATTR_USED __attribute__((used))
#endif

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

// 标记函数为纯函数（无副作用，返回值仅依赖输入）
// 编译器可优化：相同输入直接复用结果
#ifdef _MSC_VER
#define ATTR_PURE __declspec(noalias)  // MSVC无直接对应，用noalias近似
#else
#define ATTR_PURE __attribute__((pure))
#endif

// 标记函数为常量函数（比纯函数更严格，无参数依赖，返回值固定）
// 示例：数学常量计算函数
#ifdef _MSC_VER
#define ATTR_CONST __declspec(noalias)
#else
#define ATTR_CONST __attribute__((const))
#endif

// 弱符号（可被其他定义覆盖，用于库版本兼容）
#ifdef _MSC_VER
#define ATTR_WEAK __declspec(selectany)
#else
#define ATTR_WEAK __attribute__((weak))
#endif

// 标记废弃函数（调用时编译器会警告）
#ifdef _MSC_VER
#define ATTR_DEPRECATED(message) __declspec(deprecated(message))
#else
#define ATTR_DEPRECATED(message) __attribute__((deprecated(message)))
#endif

// 对齐控制（指定变量/类型的内存对齐字节数）
#define ATTR_ALIGNED(alignment) __attribute__((aligned(alignment)))

#endif  // COMMON_COMPILER_ATTRIBUTES_H
