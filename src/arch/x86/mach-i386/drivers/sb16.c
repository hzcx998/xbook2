#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/clock.h>
#include <xbook/dma.h>
#include <xbook/schedule.h>
#include <xbook/waitqueue.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define DRV_NAME "sound blaster 16"
#define DRV_VERSION "0.1"

#define DEV_NAME "sb16"

// #define DEBUG_SB16

#define SB16_POLL

// DSP commands
enum
{
    DSP_STOP_8BIT = 0xD0,
    DSP_STOP_16BIT = 0xD5,
    DSP_STOP_AFTER_8BIT = 0xDA,
    DSP_STOP_AFTER_16BIT = 0xD9,
    DSP_PLAY_8BIT = 0xC0,
    DSP_PLAY_16BIT = 0xB0,
    DSP_PAUSE_8BIT = 0xD0,
    DSP_RESUME_8BIT = 0xD4,
    DSP_PAUSE_16BIT = 0xD5,
    DSP_RESUME_16BIT = 0xD6,

    // DSP mode flags
    DSP_PLAY_AI = 0x06,
    DSP_PLAY_UNSIGNED = 0x00,
    DSP_PLAY_SIGNED = 0x10,
    DSP_PLAY_MONO = 0x00,
    DSP_PLAY_STEREO = 0x20,
};

enum
{
    SB_SET_RATE = 0x41,
};

const u16 DSP_READ = 0x22A;
const u16 DSP_WRITE = 0x22C;
const u16 DSP_STATUS = 0x22E;
const u16 DSP_R_ACK = 0x22F;

#define DMA_COUNT 10

typedef struct _device_extension
{
    struct dma_region dma_region[DMA_COUNT];
    int data_len[DMA_COUNT];
    int index_w;
    int index_r;
    int major_version;
    wait_queue_t waiters;
} device_extension_t;

void sb16_dma_start(struct dma_region *dma_region, uint32_t length);

/* Write a value to the DSP write register */
void sb16_dsp_write(u8 value)
{
    while (in8(DSP_WRITE) & 0x80)
    {
        ;
    }
    out8(DSP_WRITE, value);
}

/* Reads the value of the DSP read register */
u8 sb16_dsp_read()
{
    while (!(in8(DSP_STATUS) & 0x80))
    {
        ;
    }
    return in8(DSP_READ);
}

/* Changes the sample rate of sound output */
void sb16_set_sample_rate(uint16_t hz)
{
    sb16_dsp_write(SB_SET_RATE); // output
    sb16_dsp_write((u8)(hz >> 8));
    sb16_dsp_write((u8)hz);
    //TODO:
    // sb16_dsp_write(0x42); // input
    // sb16_dsp_write((u8)(hz >> 8));
    // sb16_dsp_write((u8)hz);
}

bool sb16_initialize(device_extension_t *extension)
{
    //irq_disable();

    out8(0x226, 1);
    mdelay(1);
    out8(0x226, 0);

    int data = sb16_dsp_read();
    if (data != 0xaa)
    {
        printk(KERN_NOTICE "sb16: sb not ready");
        return false;
    }

    // Get the version info
    sb16_dsp_write(0xe1);
    extension->major_version = sb16_dsp_read();
    int vmin = sb16_dsp_read();
    printk(KERN_INFO "sb16: found version %d.%d\n", extension->major_version, vmin);
    vmin = 0;
    return true;
}
void sb16_request(device_extension_t *extension)
{
    struct dma_region *dma_region = &extension->dma_region[extension->index_r];

    u8 mode = DSP_PLAY_SIGNED | DSP_PLAY_STEREO;
    // u8 mode = DSP_PLAY_SIGNED;

    int length = extension->data_len[extension->index_r];
    if (extension->index_r == extension->index_w)
        return;

    sb16_dma_start(dma_region, length);
    u16 sample_count = length / sizeof(int16_t);
    if (mode & DSP_PLAY_STEREO)
        sample_count /= 2;
    sample_count -= 1;

    sb16_dsp_write(DSP_PLAY_16BIT);
    sb16_dsp_write(mode);
    sb16_dsp_write((u8)sample_count);
    sb16_dsp_write((u8)(sample_count >> 8));
    #ifdef DEBUG_SB16
    printk(KERN_DEBUG "sb16: [DMA] [%x] sample_count: %d\n", dma_region->v, length);
    #endif
}

/**
 * mouse_handler - 鼠标中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
static int sb16_handler(unsigned long irq, unsigned long data)
{
    device_extension_t *extension = (device_extension_t *)data;
    #ifdef DEBUG_SB16
    struct dma_region *dma_region = &extension->dma_region[extension->index_r];
    #endif
    
    // Stop sound output ready for the next block.
    sb16_dsp_write(DSP_PAUSE_16BIT);
    // memset((void *)dma_region->v, 0, dma_region->p.size);

    in8(DSP_STATUS); // 8 bit interrupt
    if (extension->major_version >= 4)
        in8(DSP_R_ACK); // 16 bit interrupt

    extension->index_r = (extension->index_r + 1) % DMA_COUNT;
    #ifdef DEBUG_SB16
    printk(KERN_DEBUG "sb16: [READ FINISH] [%x]\n", dma_region->v);
    #endif
    wait_queue_wakeup_all(&extension->waiters);

    if (extension->index_r != extension->index_w)
    {
        #ifdef DEBUG_SB16
        printk(KERN_DEBUG "sb16: [NEW READ]\n");
        #endif
        sb16_request(extension);
    }
    return 0;
}

void sb16_dma_start(struct dma_region *dma_region, uint32_t length)
{
    const uint32_t addr = dma_region->p.address;
    const u8 channel = 5; // 16-bit samples use DMA channel 5 (on the master DMA controller)
    const u8 mode = 0;

    // Disable the DMA channel
    out8(0xd4, 4 + (channel & 3));

    // Clear the byte pointer flip-flop
    out8(0xd8, 0);

    // Write the DMA mode for the transfer
    out8(0xd6, (channel & 3) | mode);

    // Write the offset of the buffer
    u16 offset = (addr / 2) & 0xffff;
    out8(0xc4, (u8)offset);
    out8(0xc4, (u8)(offset >> 8));

    // Write the transfer length
    out8(0xc6, (u8)(length - 1));
    out8(0xc6, (u8)((length - 1) >> 8));

    // Write the buffer
    out8(0x8b, addr >> 16);

    // Enable the DMA channel
    out8(0xd4, (channel & 3));
}

static ssize_t __sb16_write(device_extension_t *extension, const u8 *data, ssize_t length)
{
    struct dma_region *dma_region = &extension->dma_region[extension->index_w];

    if (length > dma_region->p.size)
    {
        return -ENOSPC;
    }

    while (((extension->index_w + 1) % DMA_COUNT) == extension->index_r)
    {
        wait_queue_add(&extension->waiters, current_task);
        task_sleep();
    }
    extension->data_len[extension->index_w] = length;
    memcpy((void *)dma_region->v, data, length);

    #ifdef DEBUG_SB16
    printk(KERN_DEBUG "sb16: [WRITE] [%x] %d\n", dma_region->v, length);
    #endif
    if (extension->index_w == extension->index_r)
    {
        extension->index_w = (extension->index_w + 1) % DMA_COUNT;
        sb16_request(extension);
    }
    else
    {
        extension->index_w = (extension->index_w + 1) % DMA_COUNT;
    }
    return length;
}

static iostatus_t sb16_open(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;

    device_extension_t *extension = (device_extension_t *)device->device_extension;
    extension->index_r = extension->index_w = 0;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t sb16_write(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    int len = __sb16_write(device->device_extension, ioreq->user_buffer, ioreq->parame.write.length);
    if (len < 0)
    {
        status = IO_FAILED;
        printk(KERN_ERR "sb16: write fialed!\n");
    }

    ioreq->io_status.status = status;
    ioreq->io_status.infomation = len;

    io_complete_request(ioreq);
    return status;
}

iostatus_t sb16_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;

    iostatus_t status;

    switch (ctlcode)
    {
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t sb16_enter(driver_object_t *driver)
{
    iostatus_t status;
    device_object_t *devobj;
    device_extension_t *extension;

    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_SOUND, &devobj);
    if (status != IO_SUCCESS)
    {
        printk(KERN_ERR "sb16_enter: create device failed!\n");
        return status;
    }

    /* neighter io mode */
    devobj->flags = 0;

    extension = (device_extension_t *)devobj->device_extension;
    extension->index_r = extension->index_w = 0;

    for (int i = 0; i < DMA_COUNT; i++)
    {
        #ifdef DEBUG_SB16
        printk(KERN_INFO "[i:%d][count:%d]\n", i, DMA_COUNT);
        #endif
        extension->dma_region[i].p.size = PAGE_SIZE * 16; // 32 kb
        extension->dma_region[i].p.alignment = 0x1000;
        extension->dma_region[i].flags = DMA_REGION_SPECIAL; // spacil device
        if (dma_alloc_buffer(&extension->dma_region[i]) < 0)
        {
            printk(KERN_ERR "sb16_enter: alloc dma buffer failed!\n");
            io_delete_device(devobj);
            status = IO_FAILED;
            return status;
        }
        #ifdef DEBUG_SB16
        printk(KERN_INFO "sb16: alloc dma buffer vir addr %x phy addr %x\n", extension->dma_region[i].v, extension->dma_region[i].p.address);
        #endif
    }
    wait_queue_init(&extension->waiters);

    if (sb16_initialize(extension) == false)
    {
        printk(KERN_ERR "sb16_enter: create device failed!\n");
        io_delete_device(devobj);
        status = IO_FAILED;
        return status;
    }
    const int sample_rate = 44100;
    sb16_set_sample_rate(sample_rate);

    /* 注册时钟中断并打开中断，因为设定硬件过程中可能产生中断，所以要提前打开 */
    irq_register(IRQ5, sb16_handler, IRQF_DISABLED, "IRQ5", DRV_NAME, (unsigned long)extension);

    return status;
}

static iostatus_t sb16_exit(driver_object_t *driver)
{
    /* 遍历所有对象 */
    device_object_t *devobj, *next;
    device_extension_t *extension;
    /* 由于涉及到要释放devobj，所以需要使用safe版本 */
    list_for_each_owner_safe(devobj, next, &driver->device_list, list)
    {
        extension = (device_extension_t *)devobj->device_extension;
        int i;
        for (i = 0; i < DMA_COUNT; i++)
        {
            dma_free_buffer(&extension->dma_region[i]);
        }
        io_delete_device(devobj); /* 删除每一个设备 */
    }
    string_del(&driver->name); /* 删除驱动名 */
    return IO_SUCCESS;
}

iostatus_t sb16_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;

    /* 绑定驱动信息 */
    driver->driver_enter = sb16_enter;
    driver->driver_exit = sb16_exit;

    driver->dispatch_function[IOREQ_OPEN] = sb16_open;
    driver->dispatch_function[IOREQ_WRITE] = sb16_write;
    driver->dispatch_function[IOREQ_DEVCTL] = sb16_devctl;

    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_SB16
    printk(KERN_DEBUG "sb16_driver_func: driver name=%s\n",
           driver->name.text);
#endif
    return status;
}

static __init void sb16_driver_entry(void)
{
    if (driver_object_create(sb16_driver_func) < 0)
    {
        printk(KERN_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(sb16_driver_entry);
