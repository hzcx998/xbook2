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


#define SOUND_DEVICE "sb16"

static int sound_fd = 0;

static unsigned short MP3_Data[OUTPUT_BUFFER_SIZE];

unsigned short *get_framebuf()
{
	return MP3_Data;
}

void open_sound()
{
    sound_fd = open(SOUND_DEVICE, O_DEVEX);
}

void close_sound()
{
    close(sound_fd);
}

void submit_framebuf()
{
	write(sound_fd, MP3_Data, OUTPUT_BUFFER_SIZE * 2);
}