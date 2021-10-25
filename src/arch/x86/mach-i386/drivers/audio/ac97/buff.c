#include"buff.h"

snd_buffer_t * snd_buffer_create(size_t size)
{
	
    snd_buffer_t * buff=mem_alloc(sizeof(snd_buffer_t));
    buff->buffer=mem_alloc(size);
    buff->read_ptr=0;
    buff->write_ptr=0;
    buff->size=size;
    return buff;
}

void snd_buffer_destroy(snd_buffer_t * buff)
{
    mem_free(buff->buffer);
    mem_free(buff);
}

size_t snd_buffer_available(snd_buffer_t * buff)
 {
	if (buff->read_ptr == buff->write_ptr) //缓冲区空的
    {
		return buff->size - 1;
	}

	if (buff->read_ptr > buff->write_ptr)//中间无数据
    {
		return buff->read_ptr - buff->write_ptr - 1;
	}
    else //中间为数据
    {
		return (buff->size - buff->write_ptr) + buff->read_ptr - 1;
	}
}
void snd_buffer_increment_write(snd_buffer_t* buff)
{
    buff->write_ptr++;
	if (buff->write_ptr == buff->size) {
		buff->write_ptr = 0;
	}
}

size_t snd_buffer_write(snd_buffer_t * buff, size_t size, uint8_t * data)
{
    size_t written = 0;
	while (written < size)
    {

		while (snd_buffer_available(buff) > 0 && written < size)//还有空间且还没写完 
        {
			buff->buffer[buff->write_ptr] = data[written];
			snd_buffer_increment_write(buff);// 增量写指针
			written++;
		}
		if (written < size)
        {
            keprint("ac97 buff written<size ");
		}
	}
	return written;
}

size_t snd_buffer_unread(snd_buffer_t * buffer)//可读数量 
{
	if (buffer->read_ptr == buffer->write_ptr) 
    {
		return 0;
	}
	if (buffer->read_ptr > buffer->write_ptr)
    {
		return (buffer->size - buffer->read_ptr) + buffer->write_ptr;
	} 
    else 
    {
		return (buffer->write_ptr -buffer->read_ptr);
	}
}

static inline void snd_buffer_increment_read(snd_buffer_t * buff) {
	buff->read_ptr++;
	if (buff->read_ptr == buff->size) {
		buff->read_ptr = 0;
	}
}

size_t snd_buffer_read(snd_buffer_t * buff, size_t size, uint8_t * b) {
	size_t collected = 0;
	while (collected == 0)
    {
		while (snd_buffer_unread(buff) > 0 && collected < size) 
        {
			b[collected] = buff->buffer[buff->read_ptr];
			snd_buffer_increment_read(buff);
			collected++;
		}
		if (collected == 0) 
        {
		}
	}
	return collected;
}
