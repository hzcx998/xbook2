/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* xbook res */
#include <sys/res.h>
#include <sys/ioctl.h>
#include <xbook/diskman.h>
#include <xbook/debug.h>

/* 文件系统驱动映射表 */
extern int fatfs_drv_map[FF_VOLUMES];

/*
fatfs文件系统磁盘映射表
0, handle
1, handle
2, handle
3, handle
4, handle
*/

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    if (pdrv >= FF_VOLUMES)
        return STA_NOINIT;
	DSTATUS stat = 0;
    return stat;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = 0;
    if (pdrv >= FF_VOLUMES)
        return STA_NOINIT;
    if (diskman.open(fatfs_drv_map[pdrv]) < 0) {
        stat = STA_NODISK;
        kprint(PRINT_ERR "%s: open disk solt %d failed!\n", __func__, fatfs_drv_map[pdrv]);
    }
    return stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
    if (pdrv >= FF_VOLUMES)
        return RES_PARERR;

	DRESULT res = RES_OK;
    if (diskman.read(fatfs_drv_map[pdrv], sector, (void *) buff, count * FF_MIN_SS) < 0) {
        res = RES_ERROR;
    }
    return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if (pdrv >= FF_VOLUMES)
        return RES_PARERR;

	DRESULT res = RES_OK;
    if (diskman.write(fatfs_drv_map[pdrv], sector, (void *) buff, count * FF_MIN_SS) < 0) {
        res = RES_ERROR;
    }
    return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    if (pdrv >= FF_VOLUMES)
        return RES_PARERR;
	
	DRESULT res;
    switch(cmd)
    {
    case CTRL_SYNC:
        res = RES_OK;
        break;     
    case GET_SECTOR_SIZE:
        *(WORD*)buff = 512; res = RES_OK;
        break;     
    case GET_BLOCK_SIZE:
        *(WORD*)buff = 1; res = RES_OK;
        break;     
    case GET_SECTOR_COUNT:
        diskman.ioctl(fatfs_drv_map[pdrv], DISKIO_GETSIZE, (unsigned long) buff);
        res = RES_OK;
        break;
    default:
        res = RES_ERROR;
        break;
    }
	return res;
}
