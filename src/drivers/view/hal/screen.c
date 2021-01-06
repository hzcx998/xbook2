#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <xbook/virmem.h>
#include <xbook/driver.h>
#include <xbook/memspace.h>

#include "drivers/view/hal.h"

// #define DEBUG_VIEW_SCREEN

#ifndef   DEVICE_NAME
#define   DEVICE_NAME        "video"
#endif

int view_screen_open(view_screen_t *screen)
{
    int fd = device_open( DEVICE_NAME, 0 );
    if (fd < 0) {
        keprint("video device %s not found!\n", DEVICE_NAME);
        return -1;
    }
    screen->handle = fd;
    return 0;
}

int view_screen_close(view_screen_t *screen)
{
    if (!screen)
        return -1;
    device_close(screen->handle);
    screen->handle = -1;
    return 0;
}

int view_screen_map(view_screen_t *screen)
{
    if (screen->handle < 0)
        return -1;
    video_info_t video_info;
    int ret = device_devctl(screen->handle, VIDEOIO_GETINFO, (unsigned long) &video_info);
    if (ret < 0) {
        errprint("view screen: video get info failed!\n");
        return -1;
    }
    screen->width = video_info.x_resolution;
    screen->height = video_info.y_resolution;
    screen->bpp = video_info.bits_per_pixel;
#ifdef DEBUG_VIEW_SCREEN
    keprint("view screen: width %d, height %d, bpp %d\n", screen->width, screen->height, screen->bpp);
#endif
    unsigned int vram_size = video_info.bytes_per_scan_line * video_info.y_resolution;
    if (!vram_size) {
        errprint("view screen: video ram size is 0!\n");
        return -1;
    }
#ifdef DEBUG_VIEW_SCREEN
    keprint("view screen: screen memory map size %x\n", vram_size);
#endif
    unsigned char *vram = device_mmap(screen->handle, vram_size, IO_KERNEL);
    if (vram == NULL) {
        errprint("view screen: video mapped failed!\n");
        return -1;
    }
    screen->vram_start = vram;
    return 0;
}

int view_screen_unmap(view_screen_t *screen)
{
    if (screen->handle < 0)
        return -1;
    if (!screen->vram_start)
        return -1;
    memio_unmap(screen->vram_start);
    screen->vram_start = NULL;
    return 0;
}