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

#include <stdio.h>

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_IDE0		1	/* Example: Map IDE0 to physical drive 1 */
#define DEV_IDE1		2	/* Example: Map IDE1 to physical drive 2 */

/* device handle table */
int device_handles[FF_VOLUMES] = {-1, -1, -1};

extern disk_drive_t disk_drives[];


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

	case DEV_IDE0 :
		//result = MMC_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_IDE1 :
		//result = USB_disk_status();

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

    result = res_open(disk_drives[pdrv].devent.de_name, RES_DEV, 0);
    if (result >= 0) {
        disk_drives[pdrv].handle = result;
        stat = 0;
    } else {
        stat = STA_NODISK;
        printf("%s: open disk %s failed!\n", __func__, disk_drives[pdrv].devent.de_name);
    }
    return stat;

	switch (pdrv) {
	case DEV_RAM :
        result = res_open("ramdisk", RES_DEV, 0);
        if (result >= 0) {
            
            device_handles[DEV_RAM] = result;
            stat = 0;
        } else {
            stat = STA_NODISK;
            printf("open ramdisk failed!\n");
        }
        break;
	case DEV_IDE0 :
		//result = RAM_disk_initialize();
        result = res_open("ide0", RES_DEV, 0);
        if (result >= 0) {
            device_handles[DEV_IDE0] = result;
            stat = 0;
        } else {
            stat = STA_NODISK;
        }
		break;
	case DEV_IDE1 :
		//result = RAM_disk_initialize();
        result = res_open("ide1", RES_DEV, 0);
        if (result >= 0) {
            device_handles[DEV_IDE1] = result;
            stat = 0;
        } else {
            stat = STA_NODISK;
        }
		break;
    default:
        stat = STA_NOINIT;
        break;
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

    result = res_read(disk_drives[pdrv].handle, sector, buff, count * FF_MIN_SS);
    if (result < 0) {
        res = RES_ERROR;
    } else {
        res = RES_OK;
    }
    return res;

	switch (pdrv) {
	case DEV_RAM :
        result = res_read(device_handles[DEV_RAM], sector, buff, count * FF_MIN_SS);
        if (result < 0) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
		break;
	case DEV_IDE0 :
		result = res_read(device_handles[DEV_IDE0], sector, buff, count * FF_MIN_SS);
        if (result < 0) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
		break;
	case DEV_IDE1 :
        result = res_read(device_handles[DEV_IDE1], sector, buff, count * FF_MIN_SS);
        if (result < 0) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
        break;
	default:
        res = RES_PARERR;
        break;	
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

    result = res_write(disk_drives[pdrv].handle, sector, (char *) buff, count * FF_MIN_SS);
    if (result < 0) {
        res = RES_ERROR;
    } else {
        res = RES_OK;
    }
    return res;


	switch (pdrv) {
	case DEV_RAM :
        result = res_write(device_handles[DEV_RAM], sector, (char *) buff, count * FF_MIN_SS);
        if (result < 0) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
        break;
	case DEV_IDE0 :
		result = res_write(device_handles[DEV_IDE0], sector, (char *) buff, count * FF_MIN_SS);
        if (result < 0) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
		break;
	case DEV_IDE1 :
        result = res_write(device_handles[DEV_IDE1], sector, (char *) buff, count * FF_MIN_SS);
        if (result < 0) {
            res = RES_ERROR;
        } else {
            res = RES_OK;
        }
        break;
    default:
        res = RES_PARERR;
        break;
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
        *(DWORD*)buff = res_ioctl(disk_drives[pdrv].handle, DISKIO_GETSIZE, 0);res = RES_OK;
        break;
    default:
        res = RES_ERROR;
        break;
    }

    return res;
    
	switch (pdrv) {
	case DEV_RAM :
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
            *(DWORD*)buff = res_ioctl(device_handles[DEV_RAM], DISKIO_GETSIZE, 0);res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
        }
        break;
	case DEV_IDE0 :
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
            *(DWORD*)buff = res_ioctl(device_handles[DEV_IDE0], DISKIO_GETSIZE, 0);res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
        }
        break;

	case DEV_IDE1 :
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
            *(DWORD*)buff = res_ioctl(device_handles[DEV_IDE1], DISKIO_GETSIZE, 0);res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
        }
        break;
	}

	return res;
}
