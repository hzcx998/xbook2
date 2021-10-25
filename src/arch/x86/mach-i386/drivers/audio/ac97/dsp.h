#ifndef __DSP_H
#define __DSP_H
#include"buff.h"
#include"snd.h"
#include"string.h"
#include <xbook/memcache.h>
#include <xbook/debug.h>
typedef struct dsp_node {
	snd_buffer_t * rb;
	size_t samples;
	size_t written;
	int realtime;
}dsp_data_t;

#define SND_BUF_SIZE 0x4000
#endif