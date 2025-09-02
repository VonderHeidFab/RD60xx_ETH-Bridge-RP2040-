/* ffconf.h - FatFs configuration file (C) ChaN, 2022 */

#ifndef _FFCONF
#define _FFCONF 82786    /* Revision ID */

/*---------------------------------------------------------------------------/
/ Function and Buffer Configurations
/---------------------------------------------------------------------------*/
#define _FS_TINY        0   /* 0:Normal or 1:Tiny (Use smaller RAM for file object) */
#define _FS_READONLY    0   /* 0:Read/Write or 1:Read-only */
#define _FS_MINIMIZE    0   /* 0:Full features or 1..3:Reduce code size */
#define _FS_RPATH       1   /* 0:Disable relative path, 1:Enable relative path */
#define _FS_STRF_ENCODE 0   /* 0:ANSI/OEM or 1:UTF-16 support */

/*---------------------------------------------------------------------------/
/ Locale and Namespace Configurations
/---------------------------------------------------------------------------*/
#define _USE_LFN        1   /* 0:Disable LFN, 1:Enable LFN with static buffer, 2:Enable LFN with dynamic heap */
#define _MAX_LFN        255 /* Maximum LFN length */
#define _LFN_UNICODE    0   /* 0:ANSI/OEM or 1:Unicode */

/*---------------------------------------------------------------------------/
/ Drive/Volume Configurations
/---------------------------------------------------------------------------*/
#define _VOLUMES        2   /* Max number of volumes (logical drives) */
#define _STR_VOLUME_ID  0   /* 0:Disable string volume ID */
#define _MULTI_PARTITION 0  /* 0:Single partition only, 1:Multiple partitions */

/*---------------------------------------------------------------------------/
/ System Configurations
/---------------------------------------------------------------------------*/
#define _WORD_ACCESS    0   /* 0:Byte access, 1:Word access (16-bit) */
#define _FS_EXFAT       0   /* 0:Disable exFAT, 1:Enable exFAT support */
#define _FS_NORTC       1   /* 1:Disable RTC function */
#define _FS_NOFSINFO    0   /* 1:Do not use FSINFO */
#define _FS_LOCK        0   /* 0:No file lock, >0:Number of files that can be locked */

/*---------------------------------------------------------------------------/
/ Additional Functions
/---------------------------------------------------------------------------*/
#define _USE_FORWARD    1   /* 1:Enable f_forward function */
#define _USE_FASTSEEK   1   /* 1:Enable fast seek */
#define _USE_EXPAND     1   /* 1:Enable cluster pre-allocation */

/*---------------------------------------------------------------------------/
/ Sector/Buffer Configurations
/---------------------------------------------------------------------------*/
#define _MAX_SS         512 /* Max sector size */
#define _MIN_SS         512 /* Min sector size */

#endif /* _FFCONF */
