/* ff.h - FatFs module header file (C) ChaN, 2022 */

#ifndef _FF_H_
#define _FF_H_

#include <stdint.h>
#include <stddef.h>
#include "ffconf.h"

#ifndef FF_TYPES_DEFINED
#define FF_TYPES_DEFINED
typedef unsigned int UINT;
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* Type definitions */
typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t LBA_t;    /* Logical Block Address type */

typedef struct {
    DWORD    fsize;        /* File size */
    WORD     fdate;        /* Last modified date */
    WORD     ftime;        /* Last modified time */
    BYTE     fattrib;      /* File attribute */
} FILINFO;

/* File attributes */
#define AM_RDO  0x01    /* Read only */
#define AM_HID  0x02    /* Hidden */
#define AM_SYS  0x04    /* System */
#define AM_DIR  0x10    /* Directory */
#define AM_ARC  0x20    /* Archive */

/* Result code of FatFs functions */
typedef enum {
    FR_OK = 0,
    FR_DISK_ERR,
    FR_INT_ERR,
    FR_NOT_READY,
    FR_NO_FILE,
    FR_NO_PATH,
    FR_INVALID_NAME,
    FR_DENIED,
    FR_EXIST,
    FR_INVALID_OBJECT,
    FR_WRITE_PROTECTED,
    FR_INVALID_DRIVE,
    FR_NOT_ENABLED,
    FR_NO_FILESYSTEM,
    FR_MKFS_ABORTED,
    FR_TIMEOUT,
    FR_LOCKED,
    FR_NOT_ENOUGH_CORE,
    FR_TOO_MANY_OPEN_FILES
} FRESULT;

/* File object structure */
typedef struct {
    BYTE *buffer;
    DWORD fptr;           /* File read/write pointer */
    DWORD fsize;          /* File size */
    BYTE flag;            /* File status flags */
    BYTE pad;
} FIL;

/* Flags for file open mode */
#define FA_READ             0x01
#define FA_WRITE            0x02
#define FA_CREATE_ALWAYS    0x08

/* Function prototypes */
FRESULT f_mount(void* fs, const char* path, BYTE opt);
FRESULT f_open(FIL* fp, const char* path, BYTE mode);
FRESULT f_read(FIL* fp, void* buff, UINT btr, UINT* br);
FRESULT f_write(FIL* fp, const void* buff, UINT btw, UINT* bw);
FRESULT f_lseek(FIL* fp, DWORD ofs);
FRESULT f_close(FIL* fp);
FRESULT f_unlink(const char* path);
FRESULT f_mkdir(const char* path);
FRESULT f_opendir(void* dp, const char* path);
FRESULT f_readdir(void* dp, FILINFO* fno);
FRESULT f_stat(const char* path, FILINFO* fno);
FRESULT f_rename(const char* path_old, const char* path_new);

#ifdef __cplusplus
}
#endif

#endif /* _FF_H_ */
