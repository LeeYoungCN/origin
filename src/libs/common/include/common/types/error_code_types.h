#ifndef COMMON_TYPES_ERROR_CODE_H
#define COMMON_TYPES_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint64_t ErrorCode;
typedef uint16_t ErrModuleId;
typedef uint16_t ErrorType;
typedef uint16_t ErrorId;

typedef enum {
    ERR_SEV_SUCCESS = 0,  // 成功(无错误)
    ERR_SEV_INFO = 1,     // 提示(不影响流程)
    ERR_SEV_WARN = 2,     // 警告(可能有风险)
    ERR_SEV_ERROR = 3,    // 错误(功能失败, 可恢复)
    ERR_SEV_FATAL = 4     // 严重错误(系统崩溃, 不可恢复)
} ErrorSeverity;

#define ERR_SEVERITY_MASK \
    0xFFFF000000000000ULL  // 错误级别：高16位(第63~48位), 掩码0xFFFF000000000000
#define ERR_MODULE_MASK 0x0000FFFF00000000ULL  // 模块ID：中16位(第47~32位), 掩码0x0000FFFF00000000
#define ERR_TYPE_MASK \
    0x00000000FFFF0000ULL                  // 错误类型：次中16位(第31~16位), 掩码0x00000000FFFF0000
#define ERR_ID_MASK 0x000000000000FFFFULL  // 具体错误码：低16位(第15~0位), 掩码0x000000000000FFFF

// ======================== 移位量(与掩码对应, 无溢出)

#define ERR_SEVERITY_SHIFT 48  // 错误级别左移48位
#define ERR_MODULE_SHIFT   32  // 模块ID左移32位
#define ERR_TYPE_SHIFT     16  // 错误类型左移16位
#define ERR_ID_SHIFT       0   // 具体错误码无需移位

// ======================== 错误码构造宏 ========================
#define MAKE_ERROR(severity, module, type, errId)                                                  \
    ((ErrorCode)(((uint64_t)(severity) << ERR_SEVERITY_SHIFT) |                                    \
                 ((uint64_t)(module) << ERR_MODULE_SHIFT) | ((uint64_t)(type) << ERR_TYPE_SHIFT) | \
                 ((uint64_t)(errId) << ERR_ID_SHIFT)))
#ifdef __cplusplus
}
#endif
#endif  // COMMON_TYPES_ERROR_CODE_H
