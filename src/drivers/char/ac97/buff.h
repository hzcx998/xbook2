#ifndef __BUFF_H
#define __BUFF_H
#include"stdint.h"
#include <xbook/memcache.h>
#include <xbook/debug.h>

typedef struct {
	unsigned char * buffer;
	size_t write_ptr;
	size_t read_ptr;
	size_t size;
	//需要加锁
	//int discard;
} snd_buffer_t;


/**
	*@breaf   创建缓冲区。
	*@param   size[in],创建大小。
	*@return  缓冲区指针
**/
snd_buffer_t * snd_buffer_create(size_t size);

/**
	*@breaf   销毁缓冲区。
	*@param   buff[in],缓冲区指针。
**/
void snd_buffer_destroy(snd_buffer_t * buff);

/**
	*@breaf   获取可用的缓冲区大小。
	*@param   buff[in],缓冲区指针。
	*@return  可用的缓冲区大小。
**/
size_t snd_buffer_available(snd_buffer_t * buff);


/**
	*@breaf   写缓冲区。
	*@param   buff[in],缓冲区指针。
	*@param   size[in],写入长度。
	*@param   data[in],写入的数据指针。
	*@return  写入缓冲区长度。
**/
size_t snd_buffer_write(snd_buffer_t * buff, size_t size, uint8_t * data);


/**
	*@breaf   读缓冲区。
	*@param   buff[in],缓冲区指针。
	*@param   size[in],读入入长度。
	*@param   data[out],读入的数据指针。
	*@return  读缓冲区长度。
**/
size_t snd_buffer_read(snd_buffer_t * buff, size_t size, uint8_t * b); 

#endif
