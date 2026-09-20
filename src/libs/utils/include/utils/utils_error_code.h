#ifndef ORIGIN_UTILS_ERROR_CODE_H
#define ORIGIN_UTILS_ERROR_CODE_H

#include "common/common_error_code.h"
#include "common/types/error_code_types.h"

#define UTILS_MODULE_ID 2
typedef enum {
    UTILS_ERR_TYPE_DATE_TIME = 1,
    UTILS_ERR_TYPE_FILESYSTEM,
} UtilsErrType;

typedef enum {
    DT_ERR_ID_MONTH_INVALID = 1,
    DT_ERR_ID_WEEKDAY_INVALID,
    DT_ERR_ID_TIMEZONE_INVALID,
    DT_ERR_ID_TIMESTAMP_INVALID
} DateTimeErrId;

#define MAKE_DATE_TIME_ERROR(errId) \
    MAKE_ERROR(ERR_SEV_ERROR, UTILS_MODULE_ID, UTILS_ERR_TYPE_DATE_TIME, DateTimeErrId::errId)
// datetime
#define ERR_UTILS_MONTH_INVALID     MAKE_DATE_TIME_ERROR(DT_ERR_ID_MONTH_INVALID)
#define ERR_UTILS_WEEKDAY_INVALID   MAKE_DATE_TIME_ERROR(DT_ERR_ID_WEEKDAY_INVALID)
#define ERR_UTILS_TIMEZONE_INVALID  MAKE_DATE_TIME_ERROR(DT_ERR_ID_TIMEZONE_INVALID)
#define ERR_UTILS_TIMESTAMP_INVALID MAKE_DATE_TIME_ERROR(DT_ERR_ID_TIMESTAMP_INVALID)

typedef enum {
    FS_ERR_ID_NOT_FOUND = 1,      ///< 路径不存在
    FS_ERR_ID_PERMISSION_DENIED,  ///< 权限不足
    FS_ERR_ID_PATH_TOO_LONG,      ///< 路径长度超过系统限制
    FS_ERR_ID_ALREADY_EXISTS,     ///< 目标路径已存在
    FS_ERR_ID_NOT_DIRECTORY,      ///< 路径指向的不是目录
    FS_ERR_ID_NOT_FILE,           ///< 路径指向的不是文件
    FS_ERR_ID_IO_ERROR,           ///< I/O操作错误
    FS_ERR_ID_DIR_NOT_EMPTY,      ///< 文件夹非空
    FS_ERR_ID_PATH_INVALID,       ///< 路径不合法
    FS_ERR_ID_SHARING_VIOLATION,  ///< 文件共享冲突
    FS_ERR_ID_IS_A_DIRECTORY,     ///< 目标是文件夹
    FS_ERR_ID_FILE_NOT_OPEN,      ///< 文件未打开
} FileSysErrId;

#define MAKE_FILESYS_ERROR(errId) \
    MAKE_ERROR(ERR_SEV_ERROR, UTILS_MODULE_ID, UTILS_ERR_TYPE_FILESYSTEM, FileSysErrId::errId)

#define ERR_UTILS_NOT_FOUND         MAKE_FILESYS_ERROR(FS_ERR_ID_NOT_FOUND)
#define ERR_UTILS_PERMISSION_DENIED MAKE_FILESYS_ERROR(FS_ERR_ID_PERMISSION_DENIED)
#define ERR_UTILS_PATH_TOO_LONG     MAKE_FILESYS_ERROR(FS_ERR_ID_PATH_TOO_LONG)
#define ERR_UTILS_ALREADY_EXISTS    MAKE_FILESYS_ERROR(FS_ERR_ID_ALREADY_EXISTS)
#define ERR_UTILS_NOT_DIRECTORY     MAKE_FILESYS_ERROR(FS_ERR_ID_NOT_DIRECTORY)
#define ERR_UTILS_NOT_FILE          MAKE_FILESYS_ERROR(FS_ERR_ID_NOT_FILE)
#define ERR_UTILS_IO_ERROR          MAKE_FILESYS_ERROR(FS_ERR_ID_IO_ERROR)
#define ERR_UTILS_DIR_NOT_EMPTY     MAKE_FILESYS_ERROR(FS_ERR_ID_DIR_NOT_EMPTY)
#define ERR_UTILS_PATH_INVALID      MAKE_FILESYS_ERROR(FS_ERR_ID_PATH_INVALID)
#define ERR_UTILS_SHARING_VIOLATION MAKE_FILESYS_ERROR(FS_ERR_ID_SHARING_VIOLATION)
#define ERR_UTILS_IS_A_DIRECTORY    MAKE_FILESYS_ERROR(FS_ERR_ID_IS_A_DIRECTORY)
#define ERR_UTILS_FILE_NOT_OPEN     MAKE_FILESYS_ERROR(FS_ERR_ID_FILE_NOT_OPEN)

static inline const char *get_utils_err_msg(ErrorCode errCode)
{
    switch (errCode) {
        // Date time error code.
        case ERR_UTILS_MONTH_INVALID:
            return "Month invalid";
        case ERR_UTILS_WEEKDAY_INVALID:
            return "Weekday invalid";
        case ERR_UTILS_TIMEZONE_INVALID:
            return "Time zone invalid";
        case ERR_UTILS_TIMESTAMP_INVALID:
            return "Timestamp invalid";
        // Filesystem error code.
        case ERR_UTILS_NOT_FOUND:
            return "Target not found";
        case ERR_UTILS_PERMISSION_DENIED:
            return "Permission denied";
        case ERR_UTILS_PATH_TOO_LONG:
            return "Path too long";
        case ERR_UTILS_ALREADY_EXISTS:
            return "File already exists";
        case ERR_UTILS_NOT_DIRECTORY:
            return "Target not directory";
        case ERR_UTILS_NOT_FILE:
            return "Target not file";
        case ERR_UTILS_IO_ERROR:
            return "IO error";
        case ERR_UTILS_DIR_NOT_EMPTY:
            return "Directory not empty";
        case ERR_UTILS_PATH_INVALID:
            return "Path invalid";
        case ERR_UTILS_SHARING_VIOLATION:
            return "Sharing violation";
        case ERR_UTILS_IS_A_DIRECTORY:
            return "Target is a directory";
        case ERR_UTILS_FILE_NOT_OPEN:
            return "File not open";
        default:
            return get_comm_err_msg(errCode);
    };
}

#endif  // ORIGIN_UTILS_ERROR_CODE_H
