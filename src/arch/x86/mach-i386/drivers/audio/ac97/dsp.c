#include"dsp.h"


void dsp_open()
{
    dsp_data_t * dsp_data=(dsp_data_t *) mem_alloc(sizeof(dsp_data_t));
    snd_device_t * snd_dsp=(snd_device_t *)mem_alloc(sizeof(snd_device_t));
    dsp_data->rb = snd_buffer_create(SND_BUF_SIZE);
	dsp_data->samples = 0;
	dsp_data->written = 0;
	dsp_data->realtime = 0;
    snd_dsp->device=dsp_data;
    strcpy(snd_dsp->name,"dsp");
    snd_register(snd_dsp);
}

 uint32_t dsp_write(dsp_data_t * dsp, uint64_t offset, uint32_t size, uint8_t *buffer) 
{
	
	size_t s = snd_buffer_available(dsp->rb);
	size_t out;
	if (size > s && dsp->realtime)
    {
		out = snd_buffer_write(dsp->rb, s & ~0x3, buffer);
	}
    else
    {
		out = snd_buffer_write(dsp->rb, size, buffer);
	}
	dsp->written += out / 4;
	return out;
}

 void dsp_close(snd_device_t * snd_dsp) 
{
    dsp_data_t * dsp=snd_dsp->device;
    uint32_t *re=snd_unregister(snd_dsp);
	if (!re) 
    {
		return;
	}
	snd_buffer_destroy(dsp->rb);
	mem_free(dsp->rb);
	mem_free(dsp);
}