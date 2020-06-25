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

#include <stdio.h>
#include "driver.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */

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
	int result = 0;
    return stat;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_status();
		// translate the reslut code here
		return stat;
	}
	return result;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
    if (pdrv >= FF_VOLUMES)
        return STA_NOINIT;

    result = drv_open();
    if (result >= 0) {
        stat = 0;
    } else {
        stat = STA_NODISK;
        printf("%s: open disk failed!\n", __func__);
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
	DRESULT res;
	int result;

    result = drv_read(sector, buff, count);
    if (result < 0) {
        res = RES_ERROR;
    } else {
        res = RES_OK;
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
	DRESULT res;
	int result;

    result = drv_write(sector, (char *) buff, count);
    if (result < 0) {
        res = RES_ERROR;
    } else {
        res = RES_OK;
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
        if (drv_ioctl(DISKIO_SYNC, 0)) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
        break;     
    case GET_SECTOR_SIZE:
        *(WORD*)buff = 512; res = RES_OK;
        break;     
    case GET_BLOCK_SIZE:
        *(WORD*)buff = 1; res = RES_OK;
        break;     
    case GET_SECTOR_COUNT:
        if (drv_ioctl(DISKIO_GETSIZE, (unsigned long )buff)) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
        break;
    default:
        res = RES_ERROR;
        break;
    }
	return res;
}
