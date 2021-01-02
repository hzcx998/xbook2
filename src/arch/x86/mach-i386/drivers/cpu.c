#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <arch/io.h>
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
    char family_str[CPU_FAMILY_STRLEN];
	char model_str[CPU_MODEL_STRLEN];
	unsigned int family;	//系列
	unsigned int model;		//型号
	unsigned int type;		//类型
	unsigned int stepping;	//频率
	unsigned int max_cpuid;	
	unsigned int  max_cpuidex;
} device_extension_t;

#define copy_family_string(dst, src)    memcpy(dst, src, CPU_FAMILY_STRLEN)
#define copy_model_string(dst, src)     memcpy(dst, src, CPU_MODEL_STRLEN)

/**CPU family*/
char cpu_family_06H[] = "Pentium 4";
char cpu_family_0FH[] = "P6";
char cpu_family_AMD[] = "Unknown AMD Processer";
char cpu_family_AMDK5[] = "AMD K5";
char cpu_family_VIA[] = "VIA Processer";
char cpu_family_SiS[] = "SiS Processer";
char cpu_family_VMware[] = "Vmware Virtual Processor";
char cpu_family_vpc[] = "Hyper-v Virtual Processor";
char cpu_family_Unknown[] = "Unknown";

/**CPU model*/
char cpu_model_06_2AH[] = "Intel Core i7 i5 i3 2xxx";
char cpu_model_06_0FH[] = "Intel Dual-core processor";
char cpu_model_unknown[] = "Unknown";

/* Cpu end of loop */
char cpu_unknown_info[] = "Unknown";

static void cpu_driver_print(device_extension_t *extension)
{
    keprint(PRINT_INFO "CPU info:\n");
    keprint(PRINT_INFO "vendor: %s brand: %s\n", extension->vendor, extension->brand);
    keprint(PRINT_INFO "family: %s model: %s\n", extension->family_str, extension->model_str);
    keprint(PRINT_INFO "type: 0x%x stepping: 0x%x\n", extension->type, extension->stepping);
    keprint(PRINT_INFO "max cpuid: 0x%x max cpuid ext: 0x%x\n", extension->max_cpuid, extension->max_cpuidex);
}
static void cpu_driver_initialize(device_extension_t *extension)
{
	uint32_t eax, ebx, ecx, edx;
	/* 
	 * 获取vendor ID
	 * 输入: eax = 0：
	 * 返回：eax = Maximum input Value for Basic CPUID information
	 */
	cpu_do_cpuid(0x00000000, 0, &eax, &ebx, &ecx, &edx);
	extension->max_cpuid = eax;
	
	memcpy(extension->vendor    , &ebx, 4);
	memcpy(extension->vendor + 4, &edx, 4);
	memcpy(extension->vendor + 8, &ecx, 4);
	extension->vendor[12] = '\0';

	/*
    * eax == 0x800000000
    * 如果CPU支持扩展信息，则在EAX中返 >= 0x80000001的值。
    */
	cpu_do_cpuid(0x80000000, 0, &eax, &ebx, &ecx, &edx);
	extension->max_cpuidex = eax;

	//先判断是哪种厂商
	if (!strncmp(extension->vendor, "GenuineIntel", 12)) {
		
		//get version information
		cpu_do_cpuid(0x00000001, 0, &eax, &ebx, &ecx, &edx);

		extension->family   = (((eax >> 20) & 0xFF) << 4)
				+ ((eax >> 8) & 0xF);
	
		extension->model    = (((eax >> 16) & 0xF) << 4)
				+ ((eax >> 4) & 0xF);
		
		//获取stepping和type
		extension->stepping = (eax >> 0) & 0xF;
		extension->type = (eax >> 12) & 0xF;	//只取2位
		
		/*
		 * CPU family & model information
		 * get family and model string.
		 */
		if (extension->family == 0x06) {
			copy_family_string(extension->family_str, cpu_family_06H);

			if (extension->model == 0x2a) {
				copy_model_string(extension->model_str, cpu_model_06_2AH);
			} else if (extension->model == 0x0f) {
				copy_model_string(extension->model_str, cpu_model_06_0FH);
			} else {
				copy_model_string(extension->model_str, cpu_model_unknown);
			}
		} else if (extension->family == 0x0f) {
			copy_family_string(extension->family_str, cpu_family_0FH);
			copy_model_string(extension->model_str, cpu_model_unknown);
		} else {
			copy_family_string(extension->family_str, cpu_family_Unknown);
			copy_model_string(extension->model_str, cpu_model_unknown);
		}
	} else if (!strncmp(extension->vendor, "AuthenticAMD", 12)) {

		copy_family_string(extension->family_str, cpu_family_AMD);
		copy_model_string(extension->model_str, cpu_model_unknown);
	} else if (!strncmp(extension->vendor, "AMDisbetter!", 12)) {

		copy_family_string(extension->family_str, cpu_family_AMDK5);
		copy_model_string(extension->model_str, cpu_model_unknown);
	} else if (!strncmp(extension->vendor, "SiS SiS SiS ", 12)) {

		copy_family_string(extension->family_str, cpu_family_SiS);
		copy_model_string(extension->model_str, cpu_model_unknown);
	} else if (!strncmp(extension->vendor, "VIA VIA VIA ", 12)) {

		copy_family_string(extension->family_str, cpu_family_VIA);
		copy_model_string(extension->model_str, cpu_model_unknown);
	} else if (!strncmp(extension->vendor, "Microsoft Hv", 12)) {

		copy_family_string(extension->family_str, cpu_family_vpc);
		copy_model_string(extension->model_str, cpu_model_unknown);
	} else if (!strncmp(extension->vendor, "VMwareVMware", 12)) {

		copy_family_string(extension->family_str, cpu_family_VMware);
		copy_model_string(extension->model_str, cpu_model_unknown);
	} else {

		copy_family_string(extension->family_str, cpu_family_Unknown);
		copy_model_string(extension->model_str, cpu_model_unknown);
	}

	/*
    * 如果CPU支持Brand String，则在max cpu externed >= 0x80000004的值。
	* 以下获取扩展商标
    */
	if(extension->max_cpuidex >= 0x80000004){
		cpu_do_cpuid(0x80000002, 0, &eax, &ebx, &ecx, &edx);
		memcpy(extension->brand      , &eax, 4);
		memcpy(extension->brand  +  4, &ebx, 4);
		memcpy(extension->brand  +  8, &ecx, 4);
		memcpy(extension->brand  + 12, &edx, 4);
		cpu_do_cpuid(0x80000003, 0, &eax, &ebx, &ecx, &edx);
		memcpy(extension->brand  + 16, &eax, 4);
		memcpy(extension->brand  + 20, &ebx, 4);
		memcpy(extension->brand  + 24, &ecx, 4);
		memcpy(extension->brand  + 28, &edx, 4);
		cpu_do_cpuid(0x80000004, 0, &eax, &ebx, &ecx, &edx);
		memcpy(extension->brand  + 32, &eax, 4);
		memcpy(extension->brand  + 36, &ebx, 4);
		memcpy(extension->brand  + 40, &ecx, 4);
		memcpy(extension->brand  + 44, &edx, 4);
		extension->brand[49] = '\0';
		int i;
		for (i = 0; i < 49; i++){
			if (extension->brand[i] > 0x20) {
				break;
			}
		}
	}
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