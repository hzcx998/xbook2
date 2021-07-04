#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>

#define DRV_NAME "vbe-graph"
#define DRV_VERSION "0.1"

#define DEV_NAME "video"

// #define DEBUG_DRV

/* 将显存映射到内核，在内核态也可以操作显存 */
#define MAP_VRAM_TO_KERN    1

#include <drivers/vbe.h>

#define VBE_INFO_ADDR  (KERN_BASE_VIR_ADDR + VBE_BASE_INFO_ADDR)
#define VBE_MODE_ADDR  (KERN_BASE_VIR_ADDR + VBE_BASE_MODE_ADDR)

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    struct vbe_mode_info_block *mode_info;
    struct vbe_info_block *vbe_info;
    unsigned char *vir_base_addr;    /* 映射到内核的虚拟地址 */
} device_extension_t;

#ifdef DEBUG_DRV
void dump_vbe_info_block(struct vbe_info_block *info)
{
    if (info == NULL)
        return;
    keprint(PRINT_DEBUG "VBE info block:\n");
    keprint(PRINT_DEBUG "vbe signature:%c%c%c%c\n", info->vbeSignature[0],
        info->vbeSignature[1], info->vbeSignature[2], info->vbeSignature[3]);
    keprint(PRINT_DEBUG "vbe version:%x\n", info->vbeVeision);
    keprint(PRINT_DEBUG "OEM string ptr:%x\n", info->oemStringPtr);
    /* 低8位有效，其他位reserved */
    keprint(PRINT_DEBUG "capabilities:%x\n", info->capabilities[0]);
    keprint(PRINT_DEBUG "video mode ptr:%x\n", info->videoModePtr);
    keprint(PRINT_DEBUG "total memory:%d\n", info->totalMemory);
    keprint(PRINT_DEBUG "oem software reversion:%x\n", info->oemSoftwareRev);
    keprint(PRINT_DEBUG "oem vendor name ptr:%x\n", info->oemVendorNamePtr);
    keprint(PRINT_DEBUG "oem product name ptr:%x\n", info->oemProductNamePtr);
    keprint(PRINT_DEBUG "oem product reversion ptr:%x\n", info->oemProductRevPtr);

    keprint(PRINT_DEBUG "oem data:\n");
    /* 打印oemData */
    unsigned int *p = (unsigned int *) info->oemData;
    int i; 
    for (i = 0; i < 256 / 4; i++) {
        keprint("%x ", p[i]);
    }
    keprint("\n");
    
    keprint(PRINT_DEBUG "reserved (mode list):\n");
    unsigned short *q = (unsigned short *) info->reserved; 
    for (i = 0; i < 222 / 2; i++) {
        keprint("%x ", q[i]);
    }
    keprint("\n");
}

void dump_vbe_mode_info_block(struct vbe_mode_info_block *info)
{
    if (info == NULL)
        return;

    keprint(PRINT_DEBUG "VBE mode info block:\n");
    keprint(PRINT_DEBUG "mode attributes:%x\n", info->modeAttributes);
    keprint(PRINT_DEBUG "horizontal resolution in pixels:%d\n", info->xResolution);
    keprint(PRINT_DEBUG "vertical resolution in pixels:%d\n", info->yResolution);
    keprint(PRINT_DEBUG "bits per pixel:%d\n", info->bitsPerPixel);
    keprint(PRINT_DEBUG "physical address for flat memory frame buffer:%x\n", info->phyBasePtr);
    keprint(PRINT_DEBUG "bytesPerScanLine:%x\n", info->bytesPerScanLine);
}
#endif  /* DEBUG_DRV */


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
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "%s: length=%x mode len=%x\n", __func__, 
        ioreq->parame.mmap.length, extension->mode_info->bytesPerScanLine *
        extension->mode_info->yResolution);
#endif
    /* 检测参数大小 */
    if (ioreq->parame.mmap.length <= extension->mode_info->bytesPerScanLine *
            extension->mode_info->yResolution) {
        
        ioreq->io_status.infomation = (unsigned long) extension->mode_info->phyBasePtr;    /* 返回物理地址 */
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "%s: get phy addr:%x\n", __func__, ioreq->io_status.infomation);
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
        keprint(PRINT_ERR "vbe_enter: create device failed!\n");
        return status;
    }
    /* neighter io mode */
    devobj->flags = 0;
    extension = (device_extension_t *)devobj->device_extension;
    extension->device_object = devobj;

    /* 在loader中从BIOS读取VBE数据到内存，现在可以获取之 */
    extension->vbe_info = (struct vbe_info_block *)VBE_INFO_ADDR;
    extension->mode_info = (struct vbe_mode_info_block *)VBE_MODE_ADDR;
    
    // 检测版本是否满足
    if (extension->vbe_info->vbeVeision < 0x0200) {
        io_delete_device(devobj);
        keprint(PRINT_ERR "vbe: version %x not supported!\n", extension->vbe_info->vbeVeision);
        status = IO_FAILED;
        return status;
    }

#ifdef DEBUG_DRV
    
    keprint(PRINT_DEBUG "%s: %s: sizeof vbe info block %d mode block %d\n", 
        DRV_NAME, __func__, sizeof(struct vbe_info_block), sizeof(struct vbe_mode_info_block));
    
    dump_vbe_info_block(extension->vbe_info);
    dump_vbe_mode_info_block(extension->mode_info);

#endif  /* VESA_DEBUG */

    extension->vir_base_addr = NULL;
#if MAP_VRAM_TO_KERN == 1
    /* 将显存映射到内核 */
    int video_ram_size = extension->mode_info->bytesPerScanLine * extension->mode_info->yResolution;
    extension->vir_base_addr = memio_remap(extension->mode_info->phyBasePtr, video_ram_size);
    if (extension->vir_base_addr == NULL) {
        status = IO_FAILED;
        keprint(PRINT_ERR "%s: %s: memio_remap for vbe ram failed!\n", 
            DRV_NAME, __func__);
        return status;
    }
#ifdef KERN_VBE_MODE
    device_notify_to("vga-console", 0, (void*[]) {
        extension->vir_base_addr,
        &extension->mode_info->xResolution,
        &extension->mode_info->yResolution,
    });
#endif /* KERN_VBE_MODE */

#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "%s: %s: " "video ram size: %x bytes\n", 
        DRV_NAME, __func__, video_ram_size);
    keprint(PRINT_DEBUG "%s: %s: " "mapped virtual addr in kernel: %x.\n", 
        DRV_NAME, __func__, extension->vir_base_addr);
#endif  /*  DEBUG_DRV */
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

iostatus_t vbe_driver_func(driver_object_t *driver)
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
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "vbe_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void vbe_driver_entry(void)
{
    if (driver_object_create(vbe_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

notify_driver_initcall(vbe_driver_entry);
