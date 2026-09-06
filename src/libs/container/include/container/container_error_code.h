#ifndef ORIGIN_CONTAINER_ERROR_CODE_H
#define ORIGIN_CONTAINER_ERROR_CODE_H

#include "common/types/error_code_types.h"
#include "common/common_error_code.h"

#define CONTAINER_MODULE_ID 3

typedef enum {
    CONTAINER_ERR_TYPE_CONTAINER
} ContainerErrType;

typedef enum {
    CTN_ERR_ID_OVERFLOW = 1,  ///< 路径不存在
} ContainerErrId;

#define MAKE_CONTAINER_ERROR(errId) \
    MAKE_ERROR(ERR_SEV_ERROR, CONTAINER_MODULE_ID, CONTAINER_ERR_TYPE_CONTAINER, errId)

#define ERR_CONTAINER_OVERFLOW MAKE_CONTAINER_ERROR(CTN_ERR_ID_OVERFLOW)

static inline const char *get_container_err_msg(ErrorCode errCode)
{
    switch (errCode) {
        // Container error code.
        case ERR_CONTAINER_OVERFLOW:
            return "Container overflow";
        // Unknown error code.
        default:
            return get_comm_err_msg(errCode);
    };
}

#endif  // ORIGIN_CONTAINER_ERROR_CODE_H
