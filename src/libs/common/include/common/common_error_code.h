#ifndef ORIGIN_COMMON_ERROR_CODE_H
#define ORIGIN_COMMON_ERROR_CODE_H

#include "common/types/error_code_types.h"

#define COMMON_MODULE_ID 1

typedef enum {
    COMM_ERR_TYPE_COMM = 1,
    COMM_ERR_TYPE_CONTAINER
} CommErrType;

typedef enum {
    COMM_ERR_ID_SUCCESS = 0,
    COMM_ERR_ID_MALLOC_FAILED = 1,
    COMM_ERR_ID_IDX_OUT_OF_RANGE,
    COMM_ERR_ID_ENUM_INVALID,
    COMM_ERR_ID_PARAM_NULL,
    COMM_ERR_ID_PARAM_INVALID,
    COMM_ERR_ID_PARAM_OUT_OF_RANGE,
    COMM_ERR_ID_PARAM_EMPTY,
    COMM_ERR_ID_BUFFER_OVERFLOW,
    COMM_ERR_ID_SYSTEM_ERROR = 100,
    COMM_ERR_ID_GENERIC_ERROR,
    COMM_ERR_ID_UNKNOWN_ERROR
} CommErrId;

// 成功
#define ERR_COMM_SUCCESS 0

#define MAKE_COMMON_ERROR(errId) \
    MAKE_ERROR(ERR_SEV_ERROR, COMMON_MODULE_ID, COMM_ERR_TYPE_COMM, errId)

// 通用错误码
#define ERR_COMM_MALLOC_FAILED      MAKE_COMMON_ERROR(COMM_ERR_ID_MALLOC_FAILED)
#define ERR_COMM_IDX_OUT_OF_RANGE   MAKE_COMMON_ERROR(COMM_ERR_ID_IDX_OUT_OF_RANGE)
#define ERR_COMM_ENUM_INVALID       MAKE_COMMON_ERROR(COMM_ERR_ID_ENUM_INVALID)
#define ERR_COMM_PARAM_NULL         MAKE_COMMON_ERROR(COMM_ERR_ID_PARAM_NULL)
#define ERR_COMM_PARAM_INVALID      MAKE_COMMON_ERROR(COMM_ERR_ID_PARAM_INVALID)
#define ERR_COMM_PARAM_OUT_OF_RANGE MAKE_COMMON_ERROR(COMM_ERR_ID_PARAM_OUT_OF_RANGE)
#define ERR_COMM_PARAM_EMPTY        MAKE_COMMON_ERROR(COMM_ERR_ID_PARAM_EMPTY)
#define ERR_COMM_BUFFER_OVERFLOW    MAKE_COMMON_ERROR(COMM_ERR_ID_BUFFER_OVERFLOW)
#define ERR_COMM_SYSTEM_ERROR       MAKE_COMMON_ERROR(COMM_ERR_ID_SYSTEM_ERROR)
#define ERR_COMM_GENERIC_ERROR      MAKE_COMMON_ERROR(COMM_ERR_ID_GENERIC_ERROR)
#define ERR_COMM_UNKNOWN_ERROR      MAKE_COMMON_ERROR(COMM_ERR_ID_UNKNOWN_ERROR)

typedef enum {
    CTN_ERR_ID_OVERFLOW = 1,  ///< 路径不存在
} ContainerErrId;

#define MAKE_CONTAINER_ERROR(errId) \
    MAKE_ERROR(ERR_SEV_ERROR, COMMON_MODULE_ID, COMM_ERR_TYPE_CONTAINER, errId)

#define ERR_COMM_CTN_OVERFLOW MAKE_CONTAINER_ERROR(CTN_ERR_ID_OVERFLOW)

static inline const char *get_comm_err_msg(ErrorCode errCode)
{
    switch (errCode) {
        // Success.
        case ERR_COMM_SUCCESS:
            return "success";
        // Common error code.
        case ERR_COMM_MALLOC_FAILED:
            return "malloc failed";
        case ERR_COMM_IDX_OUT_OF_RANGE:
            return "Input index array out of bound";
        case ERR_COMM_ENUM_INVALID:
            return "Enum invalid";
        case ERR_COMM_PARAM_NULL:
            return "Input param NULL";
        case ERR_COMM_PARAM_INVALID:
            return "Input param invalid";
        case ERR_COMM_PARAM_EMPTY:
            return "Param empty";
        case ERR_COMM_BUFFER_OVERFLOW:
            return "Buffer overflow";
        case ERR_COMM_SYSTEM_ERROR:
            return "System error";
        case ERR_COMM_GENERIC_ERROR:
            return "Generic error";
        case ERR_COMM_UNKNOWN_ERROR:
            return "Unkown error";
        // Container error code.
        case ERR_COMM_CTN_OVERFLOW:
            return "Container overflow";
        // Unknown error code.
        default:
            return "Unknown error code";
    };
}

#endif  // ORIGIN_COMMON_ERROR_CODE_H
