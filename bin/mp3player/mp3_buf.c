#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mp3_buf.h"


#define SOUND_DEVICE "/dev/sb16"

static int sound_fd = 0;

static unsigned short MP3_Data[OUTPUT_BUFFER_SIZE];

unsigned short *get_framebuf()
{
	return MP3_Data;
}

void open_sound()
{
    sound_fd = open(SOUND_DEVICE, 0);
}

void close_sound()
{
    if (sound_fd >= 0)
        close(sound_fd);
}

void submit_framebuf()
{
    if (sound_fd >= 0)
    	write(sound_fd, MP3_Data, OUTPUT_BUFFER_SIZE * 2);
}
