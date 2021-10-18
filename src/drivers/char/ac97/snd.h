#ifndef __SND_H
#define __SND_H

#include"stdint.h"
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include"buff.h"
#include"math.h"
#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>
#include"dsp.h"

#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>
#include <stdio.h>
typedef struct snd_device
{
	char name[256];            
	void * device;      //私有数据
	uint32_t playback_speed;  
	int (*mixer_read)(uint32_t knob_id, uint32_t *val);
	int (*mixer_write)(uint32_t knob_id, uint32_t val);
	uint32_t id;
} snd_device_t;



/**
	*@breaf   注册一个设备到声卡。
	*@param   device[in],设备指针。
	*@return  0成功，-1出错
**/
int snd_register(snd_device_t * device);

/**
	*@breaf   卸载一个设备到声卡。
	*@param   device[in],设备指针。
	*@return  0成功，-1出错
**/
int snd_unregister(snd_device_t * device);


/**
	*@breaf   设备申请缓冲区进行播放。
	*@param   device[in],设备指针。
	*@param   size  [in],设备需要的缓冲区大小。
	*@param   buffer[in],设备申请存放数据的缓冲区。
	*@return  返回申请到的缓冲区大小
**/
int snd_request_buf(snd_device_t * device, uint32_t size, uint8_t *buffer);



#endif
