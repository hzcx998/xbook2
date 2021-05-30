#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define DRV_NAME "processer"
#define DRV_VERSION "0.1"

#define DEV_NAME "cpu0"

#define DEBUG_CPU_DRV

#define CPU_VENDOR_STRLEN   16
#define CPU_BRAND_STRLEN   50
#define CPU_FAMILY_STRLEN   50
#define CPU_MODEL_STRLEN    50

/*
 * cpuid 分为2组信息，一组为基本信息和扩展信息，往eax输入0到3是基本信息，
 * 输入0x80000000到0x80000004返回扩展信息
 */
typedef struct _device_extension {
    char vendor[CPU_VENDOR_STRLEN];
	char brand[CPU_BRAND_STRLEN];
} device_extension_t;

static void cpu_driver_print(device_extension_t *extension)
{
    keprint(PRINT_INFO "CPU info:\n");
    keprint(PRINT_INFO "vendor: %s brand: %s\n", extension->vendor, extension->brand);
}

static void cpu_driver_initialize(device_extension_t *extension)
{
	strcpy(extension->vendor, "RISCV64");
#ifdef QEMU
    strcpy(extension->brand, "QEMU RISC-V Core 64bit");
#else
    strcpy(extension->brand, "K210 RISC-V Dual Core 64bit, with FPU");
#endif
}

static iostatus_t cpu_driver_read(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    device_extension_t *extension = (device_extension_t *) device->device_extension;
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "null_read: data:\n");
#endif
    int len = -1;
    unsigned char *data = (unsigned char *) ioreq->user_buffer;
    
    // brand
    len = min(ioreq->parame.read.length, CPU_BRAND_STRLEN);
    memcpy(data, extension->brand, len);

    ioreq->io_status.infomation = len;
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t cpu_driver_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *extension;

    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_VIRTUAL_CHAR, &devobj);

    if (status != IO_SUCCESS) {
        keprint(PRINT_ERR "cpu_driver_enter: create device failed!\n");
        return status;
    }
    /* neighter io mode */
    devobj->flags = 0;
    extension = (device_extension_t *)devobj->device_extension;
    
    cpu_driver_initialize(extension);

    /* 打印CPU信息 */
    cpu_driver_print(extension);
    return status;
}

static iostatus_t cpu_driver_exit(driver_object_t *driver)
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

static iostatus_t cpu_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = cpu_driver_enter;
    driver->driver_exit = cpu_driver_exit;


    driver->dispatch_function[IOREQ_READ] = cpu_driver_read;

    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "cpu_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void cpu_driver_entry(void)
{
    if (driver_object_create(cpu_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(cpu_driver_entry);