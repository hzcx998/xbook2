#include"snd.h"

#include"snd_list.h"


#define DRV_NAME "snd"
#define DRV_VERSION "0.1"
#define DEV_NAME "snd"


#define SND_BUF_SIZE 0x4000
#define	MIN(a,b)	((a) < (b) ? (a) : (b))
#define N_ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))

static snd_list_t _device;
typedef struct  _snd
{
    snd_list_t device;
    uint32_t device_id;
}_snd_t;

static _snd_t SND;

int snd_register(snd_device_t * device)
{
    
	device->id = SND.device_id++;
	if (snd_list_find(device, &SND.device))
    {
		return -1;
	}
	snd_list_add_tail(device, &SND.device);
	return 0;
}


int snd_unregister(snd_device_t * device) 
{
	uint32_t * re = snd_list_find(device, &SND.device);
	if (!re) 
    {
		return -1;
	}
	snd_list_del(device);
	return 0;
}


int snd_request_buf(snd_device_t * device, uint32_t size, uint8_t *buffer)
{
	static int16_t tmp_buf[0x100];
	memset(buffer, 0, size);

    snd_list_t *node;
   snd_list_for_each(node, &SND.device)
   {
	   snd_device_t *snd_device= SND.device.snd_device;
	   if(strcmp(snd_device->name,"dsp")!=0)
	   {
		   break;
	   }
	   dsp_data_t * dsp=snd_device->device;
	   snd_buffer_t * buf = dsp->rb;
		size_t bytes_left = MIN(snd_buffer_unread(buf) & ~0x3, size);
		int16_t * adding_ptr = (int16_t *) buffer;
		while (bytes_left) 
        {
			size_t this_read_size = MIN(bytes_left, sizeof(tmp_buf));
			snd_buffer_read(buf, this_read_size, (uint8_t *)tmp_buf);
			dsp->samples += this_read_size / 4; //16位双通道

			for (size_t i = 0; i < N_ELEMENTS(tmp_buf); i++) {
				tmp_buf[i] /= 2;
			}
			for (size_t i = 0; i < this_read_size / sizeof(*adding_ptr); i++) {
				adding_ptr[i] += tmp_buf[i];
			}
			adding_ptr += this_read_size / sizeof(*adding_ptr);
			bytes_left -= this_read_size;
		}

   }
	return size;
}



iostatus_t snd_write(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = ioreq->parame.write.length;
    io_complete_request(ioreq);
    return status;
}


static iostatus_t snd_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    /* 初始化一些其它内容 */
    status = io_create_device(driver, 0, DEV_NAME, DEVICE_TYPE_VIRTUAL_CHAR, &devobj);
    if (status != IO_SUCCESS) {
        keprint(PRINT_ERR "null_enter: create device failed!\n");
        return status;
    }
    /* neighter io mode */
    devobj->flags = 0;
    return status;
}

static iostatus_t snd_exit(driver_object_t *driver)
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


iostatus_t snd_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    /* 绑定驱动信息 */
    driver->driver_enter = snd_enter;
    driver->driver_exit = snd_exit;
    driver->dispatch_function[IOREQ_WRITE] = snd_write;
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "null_driver_func: driver name=%s\n",
        driver->name.text);
#endif 
    return status;
}

static __init void snd_driver_entry(void)
{
    if (driver_object_create(snd_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(snd_driver_entry);
