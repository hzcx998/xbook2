#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>
#include <xbook/vine.h>
#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/vmarea.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>

#define DRV_NAME "vbe-graph"
#define DRV_VERSION "0.1"

#define DEV_NAME "video"

#define DEBUG_LOCAL 0

/* 将显存映射到内核，在内核态也可以操作显存 */
#define MAP_VRAM_TO_KERN    1

/* VBE信息的内存地址 */
#define VBE_INFO_ADDR  0x80001100
#define VBE_MODE_ADDR  0x80001300

/* VBE信息块结构体 */
struct vbe_info_block {
    uint8_t vbeSignature[4];       /* VEB Signature: 'VESA' */
    uint16_t vbeVeision;            /* VEB Version:0300h */
    uint32_t oemStringPtr;         /* VbeFarPtr to OEM string */
    uint8_t capabilities[4];       /* Capabilities of graphics controller */
    uint32_t videoModePtr;         /* VbeFarPtr to VideoModeList */
    uint16_t totalMemory;           /* Number of 64kb memory blocks added for VEB2.0+ */
    uint16_t oemSoftwareRev;        /* VEB implementation Software revision */
    uint32_t oemVendorNamePtr;     /* VbeFarPtr to Vendor Name String */
    uint32_t oemProductNamePtr;    /* VbeFarPtr to Product Name String */
    uint32_t oemProductRevPtr;     /* VbeFarPtr to Product Revision String */
    uint8_t reserved[222];         /* Reserved for VBE implementation scratch area */
    uint8_t oemData[256];          /* Data Area for OEM String */
} __attribute__ ((packed));

struct vbe_mode_info_block {
    /* Mandatory information for all VBE revisions */
    uint16_t modeAttributes;        /* mode attributes */
    uint8_t winAAttributes;        /* window A attributes */
    uint8_t winBAttributes;        /* window B attributes */
    uint16_t winGranulaity;         /* window granulaity */
    uint16_t winSize;               /* window size */
    uint16_t winASegment;           /* window A start segment */
    uint16_t winBSegment;           /* window B start segment */
    uint32_t winFuncPtr;           /* real mode pointer to window function */
    uint16_t bytesPerScanLine;      /* bytes per scan line */
    /* Mandatory information for VBE1.2 and above */
    uint16_t xResolution;           /* horizontal resolution in pixels or characters */
    uint16_t yResolution;           /* vertical resolution in pixels or characters */
    uint8_t xCharSize;             /* character cell width in pixels */
    uint8_t yCharSize;             /* character cell height in pixels */
    uint8_t numberOfPlanes;        /* number of banks */
    uint8_t bitsPerPixel;          /* bits per pixel */
    uint8_t numberOfBanks;         /* number of banks */
    uint8_t memoryModel;           /* memory model type */
    uint8_t bankSize;              /* bank size in KB */
    uint8_t numberOfImagePages;    /* number of images */
    uint8_t reserved0;             /* reserved for page function: 1 */
    uint8_t redMaskSize;           /* size of direct color red mask in bits */
    uint8_t redFieldPosition;      /* bit position of lsb of red mask */
    uint8_t greenMaskSize;         /* size of direct color green mask in bits */
    uint8_t greenFieldPosition;    /* bit position of lsb of green mask */
    uint8_t blueMaskSize;          /* size of direct color blue mask in bits */
    uint8_t blueFieldPosition;     /* bit position of lsb of blue mask */
    uint8_t rsvdMaskSize;          /* size of direct color reserved mask in bits */
    uint8_t rsvdFieldPosition;     /* bit position of lsb of reserved mask */
    uint8_t directColorModeInfo;   /* direct color mode attributes */
    
    /* Mandatory information for VBE2.0 and above */
    uint32_t phyBasePtr;           /* physical address for flat memory frame buffer */
    uint32_t reserved1;            /* reserved-always set to 0 */
    uint16_t reserved2;             /* reserved-always set to 0 */
    /* Mandatory information for VBE3.0 and above */
    uint16_t linebytesPerScanLine;  /* bytes per scan line for linear modes */
    uint8_t bnkNumberOfImagePages; /* number of images for banked modes */
    uint8_t linNumberOfImagePages; /* number of images for linear modes */
    uint8_t linRedMaskSize;        /* size of direct color red mask(linear modes) */
    uint8_t linRedFieldPosition;   /* bit position of lsb of red mask(linear modes) */
    uint8_t linGreenMaskSize;      /* size of direct color green mask(linear modes) */
    uint8_t linGreenFieldPosition; /* bit position of lsb of green mask(linear modes) */
    uint8_t linBlueMaskSize;       /* size of direct color blue mask(linear modes) */
    uint8_t linBlueFieldPosition;  /* bit position of lsb of blue mask(linear modes) */
    uint8_t linRsvdMaskSize;       /* size of direct color reserved mask(linear modes) */
    uint8_t linRsvdFieldPosition;  /* bit position of lsb of reserved mask(linear modes) */
    uint32_t maxPixelClock;        /* maximum pixel clock (in HZ) for graphics mode */
    uint8_t reserved3[189];        /* remainder of ModeInfoBlock */
}  __attribute__ ((packed));

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    struct vbe_mode_info_block *mode_info;
    struct vbe_info_block *vbe_info;
    unsigned char *vir_base_addr;    /* 映射到内核的虚拟地址 */
} device_extension_t;

/*
mmap(int res, size_t length, int flags);
munmap(void *addr, size_t length);
*/

#if DEBUG_LOCAL == 1
void dump_vbe_info_block(struct vbe_info_block *info)
{
    if (info == NULL)
        return;
    printk(KERN_DEBUG "VBE info block:\n");
    printk(KERN_DEBUG "vbe signature:%c%c%c%c\n", info->vbeSignature[0],
        info->vbeSignature[1], info->vbeSignature[2], info->vbeSignature[3]);
    printk(KERN_DEBUG "vbe version:%x\n", info->vbeVeision);
    printk(KERN_DEBUG "OEM string ptr:%x\n", info->oemStringPtr);
    /* 低8位有效，其他位reserved */
    printk(KERN_DEBUG "capabilities:%x\n", info->capabilities[0]);
    printk(KERN_DEBUG "video mode ptr:%x\n", info->videoModePtr);
    printk(KERN_DEBUG "total memory:%d\n", info->totalMemory);
    printk(KERN_DEBUG "oem software reversion:%x\n", info->oemSoftwareRev);
    printk(KERN_DEBUG "oem vendor name ptr:%x\n", info->oemVendorNamePtr);
    printk(KERN_DEBUG "oem product name ptr:%x\n", info->oemProductNamePtr);
    printk(KERN_DEBUG "oem product reversion ptr:%x\n", info->oemProductRevPtr);

    printk(KERN_DEBUG "oem data:\n");
    /* 打印oemData */
    unsigned int *p = (unsigned int *) info->oemData;
    int i; 
    for (i = 0; i < 256 / 4; i++) {
        printk("%x ", p[i]);
    }
    printk("\n");
    
    printk(KERN_DEBUG "reserved (mode list):\n");
    unsigned short *q = (unsigned short *) info->reserved; 
    for (i = 0; i < 222 / 2; i++) {
        printk("%x ", q[i]);
    }
    printk("\n");
}

void dump_vbe_mode_info_block(struct vbe_mode_info_block *info)
{
    if (info == NULL)
        return;

    printk(KERN_DEBUG "VBE mode info block:\n");
    printk(KERN_DEBUG "mode attributes:%x\n", info->modeAttributes);
    printk(KERN_DEBUG "horizontal resolution in pixels:%d\n", info->xResolution);
    printk(KERN_DEBUG "vertical resolution in pixels:%d\n", info->yResolution);
    printk(KERN_DEBUG "bits per pixel:%d\n", info->bitsPerPixel);
    printk(KERN_DEBUG "physical address for flat memory frame buffer:%x\n", info->phyBasePtr);
    printk(KERN_DEBUG "bytesPerScanLine:%x\n", info->bytesPerScanLine);
}
#endif  /* DEBUG_LOCAL */


iostatus_t vbe_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;
    device_extension_t *extension;

    extension = device->device_extension;

    iostatus_t status;
    video_info_t *video;
    switch (ctlcode)
    {
    case VIDEOIO_GETINFO:
        video = (video_info_t *) ioreq->parame.devctl.arg;
        if (video) {
            video->bits_per_pixel = extension->mode_info->bitsPerPixel;
            video->bytes_per_scan_line = extension->mode_info->bytesPerScanLine;
            video->x_resolution = extension->mode_info->xResolution;
            video->y_resolution = extension->mode_info->yResolution;    
        }
        status = IO_SUCCESS;
        break;
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

iostatus_t vbe_mmap(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension;
    iostatus_t status = IO_FAILED;;
    
    extension = device->device_extension;

    ioreq->io_status.infomation = 0;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: length=%x mode len=%x\n", __func__, 
        ioreq->parame.mmap.length, extension->mode_info->bytesPerScanLine *
        extension->mode_info->yResolution);
#endif
    /* 检测参数大小 */
    if (ioreq->parame.mmap.length <= extension->mode_info->bytesPerScanLine *
            extension->mode_info->yResolution) {
        
        ioreq->io_status.infomation = (unsigned long) extension->mode_info->phyBasePtr;    /* 返回物理地址 */
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "%s: get phy addr:%x\n", __func__, ioreq->io_status.infomation);
#endif    
        status = IO_SUCCESS;
    }

    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t vbe_enter(driver_object_t *driver)
{
    iostatus_t status;
    device_extension_t *extension;
    device_object_t *devobj;
    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_VIDEO, &devobj);
    if (status != IO_SUCCESS) {
        printk(KERN_ERR "vbe_enter: create device failed!\n");
        return status;
    }
    /* neighter io mode */
    devobj->flags = 0;
    extension = (device_extension_t *)devobj->device_extension;
    extension->device_object = devobj;

    /* 在loader中从BIOS读取VBE数据到内存，现在可以获取之 */
    extension->vbe_info = (struct vbe_info_block *)VBE_INFO_ADDR;
    extension->mode_info = (struct vbe_mode_info_block *)VBE_MODE_ADDR;
    
#if DEBUG_LOCAL == 1
    
    printk(KERN_DEBUG "%s: %s: sizeof vbe info block %d mode block %d\n", 
        DRV_NAME, __func__, sizeof(struct vbe_info_block), sizeof(struct vbe_mode_info_block));
    
    dump_vbe_info_block(extension->vbe_info);
    dump_vbe_mode_info_block(extension->mode_info);

#endif  /* VESA_DEBUG */

    extension->vir_base_addr = NULL;
#if MAP_VRAM_TO_KERN == 1
    /* 将显存映射到内核 */
    int video_ram_size = extension->mode_info->bytesPerScanLine * extension->mode_info->yResolution;
    extension->vir_base_addr = ioremap(extension->mode_info->phyBasePtr, video_ram_size);
    if (extension->vir_base_addr == NULL) {
        status = IO_FAILED;
        printk(KERN_ERR "%s: %s: ioremap for vbe ram failed!\n", 
            DRV_NAME, __func__);
        return status;
    }
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: %s: " "video ram size: %x bytes\n", 
        DRV_NAME, __func__, video_ram_size);
    printk(KERN_DEBUG "%s: %s: " "mapped virtual addr in kernel: %x.\n", 
        DRV_NAME, __func__, extension->vir_base_addr);
#endif  /*  DEBUG_LOCAL */
#endif  /* MAP_VRAM_TO_KERN */
 
    return status;
}

static iostatus_t vbe_exit(driver_object_t *driver)
{
    /* 遍历所有对象 */
    device_object_t *devobj, *next;
    /* 由于涉及到要释放devobj，所以需要使用safe版本 */
    list_for_each_owner_safe (devobj, next, &driver->device_list, list) {
        io_delete_device(devobj);   /* 删除每一个设备 */
    }
    string_del(&driver->name); /* 删除驱动名 */
    return IO_SUCCESS;
}

iostatus_t vbe_driver_vine(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = vbe_enter;
    driver->driver_exit = vbe_exit;

    
    driver->dispatch_function[IOREQ_DEVCTL] = vbe_devctl;
    driver->dispatch_function[IOREQ_MMAP] = vbe_mmap;
    
    /*
    driver->dispatch_function[IOREQ_READ] = vbe_read;
    driver->dispatch_function[IOREQ_WRITE] = vbe_write;
    */
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "vbe_driver_vine: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}
