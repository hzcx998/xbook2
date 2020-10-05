#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/vmarea.h>
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

enum SampleFormat {
    Signed = 0x10,
    Stereo = 0x20,
};

const u16 DSP_READ = 0x22A;
const u16 DSP_WRITE = 0x22C;
const u16 DSP_STATUS = 0x22E;
const u16 DSP_R_ACK = 0x22F;

typedef struct _device_extension {
    struct dma_region dma_region;

    int major_version;
    #ifdef SB16_POLL
    int done;
    #else
    wait_queue_t waiters;
    #endif
} device_extension_t;


/* Write a value to the DSP write register */
void sb16_dsp_write(u8 value)
{
    while (in8(DSP_WRITE) & 0x80) {
        ;
    }
    out8(DSP_WRITE, value);
}

/* Reads the value of the DSP read register */
u8 sb16_dsp_read()
{
    while (!(in8(DSP_STATUS) & 0x80)) {
        ;
    }
    return in8(DSP_READ);
}

/* Changes the sample rate of sound output */
void sb16_set_sample_rate(uint16_t hz)
{
    sb16_dsp_write(0x41); // output
    sb16_dsp_write((u8)(hz >> 8));
    sb16_dsp_write((u8)hz);
    sb16_dsp_write(0x42); // input
    sb16_dsp_write((u8)(hz >> 8));
    sb16_dsp_write((u8)hz);
}

bool sb16_initialize(device_extension_t *extension)
{
    //disable_irq();

    out8(0x226, 1);
    mdelay(1);
    out8(0x226, 0);

    int data = sb16_dsp_read();
    if (data != 0xaa) {
        printk("sb16: sb not ready");
        return false;
    }

    // Get the version info
    sb16_dsp_write(0xe1);
    extension->major_version = sb16_dsp_read();
    int vmin = sb16_dsp_read();
    printk("sb16: found version %d.%d\n", extension->major_version, vmin);
    return true;
}

/**
 * mouse_handler - 鼠标中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
static int sb16_handler(unsigned long irq, unsigned long data)
{
    #ifdef DEBUG_SB16
    printk("sb16: intr occur!\n");
    #endif
    device_extension_t *extension = (device_extension_t *) data;
    // Stop sound output ready for the next block.
    sb16_dsp_write(0xd5);

    in8(DSP_STATUS); // 8 bit interrupt
    if (extension->major_version >= 4)
        in8(DSP_R_ACK); // 16 bit interrupt

    #ifdef DEBUG_SB16
    printk("sb16: intr occur done!\n");
    #endif
    #ifdef SB16_POLL
    extension->done = 1;
    #else
    wait_queue_wakeup_all(&extension->waiters);
    #endif
    return 0;
}

void sb16_dma_start(device_extension_t *extension, uint32_t length)
{
    const uint32_t addr = extension->dma_region.p.address;
    const u8 channel = 5; // 16-bit samples use DMA channel 5 (on the master DMA controller)
    const u8 mode = 0;

    // Disable the DMA channel
    out8(0xd4, 4 + (channel % 4));

    // Clear the byte pointer flip-flop
    out8(0xd8, 0);

    // Write the DMA mode for the transfer
    out8(0xd6, (channel % 4) | mode);

    // Write the offset of the buffer
    u16 offset = (addr / 2) % 65536;
    out8(0xc4, (u8)offset);
    out8(0xc4, (u8)(offset >> 8));

    // Write the transfer length
    out8(0xc6, (u8)(length - 1));
    out8(0xc6, (u8)((length - 1) >> 8));

    // Write the buffer
    out8(0x8b, addr >> 16);

    // Enable the DMA channel
    out8(0xd4, (channel % 4));
}

ssize_t __sb16_write(device_extension_t *extension, const u8* data, ssize_t length)
{

#ifdef DEBUG_SB16
    printk("sb16: Writing buffer of %d bytes\n", length);
#endif
    ASSERT(length <= PAGE_SIZE);
    const int SB16_BLOCK_SIZE = 32 * 1024;
    if (length > SB16_BLOCK_SIZE) {
        return -ENOSPC;
    }

    u8 mode = (u8)Signed | (u8)Stereo;
    
    memcpy((void *)extension->dma_region.v, data, length);
    #ifdef DEBUG_SB16
    printk("sb16: copy buffer to %x\n", extension->dma_region.v);
    #endif

    sb16_dma_start(extension, length);

    // 16-bit single-cycle output.
    // FIXME: Implement auto-initialized output.
    u8 command = 0xb0;

    u16 sample_count = length / sizeof(int16_t);
    if (mode & (u8)Stereo)
        sample_count /= 2;

    sample_count -= 1;
    #ifdef SB16_POLL
    extension->done = 0;
    #else
    disable_intr();
    disable_irq(5);
    #endif

    sb16_dsp_write(command);
    sb16_dsp_write(mode);
    sb16_dsp_write((u8)sample_count);
    sb16_dsp_write((u8)(sample_count >> 8));

    #ifdef DEBUG_SB16
    printk("sb16: set buffer done\n");
    #endif

    #ifdef SB16_POLL
    while (!extension->done) loop_delay(100);
    #else
    wait_queue_add(&extension->waiters, current_task);
    enable_irq(5);
    task_sleep();
    #endif

    #ifdef DEBUG_SB16
    printk("sb16: play done\n");
    #endif
    return length;
}



static iostatus_t rtl8139_open(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;

    
    io_complete_request(ioreq);
    return status;
}

static iostatus_t sb16_write(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    int len = __sb16_write(device->device_extension, ioreq->user_buffer, ioreq->parame.write.length);
    if (len < 0) {
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
    if (status != IO_SUCCESS) {
        printk(KERN_ERR "sb16_enter: create device failed!\n");
        return status;
    }

    /* neighter io mode */
    devobj->flags = 0;

    extension = (device_extension_t *)devobj->device_extension;
    
    extension->dma_region.p.size = PAGE_SIZE * 8; // 32 kb
    extension->dma_region.p.alignment = 0x1000;
    extension->dma_region.flags = DMA_REGION_SPECIAL; // spacil device
    if (alloc_dma_buffer(&extension->dma_region) < 0) {
        printk(KERN_ERR "sb16_enter: alloc dma buffer failed!\n");
        io_delete_device(devobj);
        status = IO_FAILED;
        return status;
    }
    printk(KERN_INFO "sb16: alloc dma buffer vir addr %x phy addr %x\n", extension->dma_region.v, extension->dma_region.p.address);

    #ifdef SB16_POLL
    extension->done = 0;
    #else
    wait_queue_init(&extension->waiters);
    #endif
    if (sb16_initialize(extension) == false) {
        printk(KERN_ERR "sb16_enter: create device failed!\n");
        io_delete_device(devobj);
        status = IO_FAILED;
        return status;
    }
    const int sample_rate = 44100;
    sb16_set_sample_rate(sample_rate);

    /* 注册时钟中断并打开中断，因为设定硬件过程中可能产生中断，所以要提前打开 */	
	register_irq(IRQ5, sb16_handler, IRQF_DISABLED, "IRQ5", DRV_NAME, (unsigned long )extension);

    return status;
}

static iostatus_t sb16_exit(driver_object_t *driver)
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

iostatus_t sb16_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = sb16_enter;
    driver->driver_exit = sb16_exit;

    
    driver->dispatch_function[IOREQ_OPEN] = rtl8139_open;
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
    if (driver_object_create(sb16_driver_func) < 0) {
        printk(KERN_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(sb16_driver_entry);
