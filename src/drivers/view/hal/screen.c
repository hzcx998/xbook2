#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <xbook/virmem.h>
#include <xbook/driver.h>
#include <xbook/memspace.h>

#include "drivers/view/hal.h"

// #define VIEW_BROWER

#ifndef   DEVICE_NAME
#define   DEVICE_NAME        "video"
#endif

int view_screen_open(view_screen_t *screen)
{
    video_info_t video_info;
    int fd = device_open( DEVICE_NAME, 0 );
    if (fd < 0) {
        keprint("video device not found!\n");
        return -1;
    }
    int ret = device_devctl(fd, VIDEOIO_GETINFO, (unsigned long) &video_info);
    if (ret < 0) {
        device_close(fd);
        return -1;
    }
    screen->width = video_info.x_resolution;
    screen->height = video_info.y_resolution;
    screen->bpp = video_info.bits_per_pixel;
#ifdef VIEW_BROWER
    keprint("xbrower: width %d, height %d, bpp %d\n", screen->width, screen->height, screen->bpp);
#endif
    unsigned int vram_size = video_info.bytes_per_scan_line * video_info.y_resolution;
    if (!vram_size) {
        device_close(fd);
        return -1;
    }
#ifdef VIEW_BROWER
    keprint("xbrower: screen memory map size %x\n", vram_size);
#endif
    unsigned char *vram = device_mmap(fd, vram_size, IO_KERNEL);
    if (vram == NULL) {
        keprint("xbrower: video mapped failed!\n");
        device_close(fd);
        return -1;
    }
    keprint("view: screen memory map at %x size %x\n", vram, vram_size);
    screen->vram_start = vram;
    screen->handle = fd;
    return 0;
}

int view_screen_close(view_screen_t *screen)
{
    if (!screen)
        return -1;
    memio_unmap(screen->vram_start);
    device_close(screen->handle);
    screen->vram_start = NULL;
    screen->handle = -1;
    return 0;
}
