#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>

#include "xgui_hal.h"

#define XGUI_DEBUG

#ifndef   DEVICE_NAME
#define   DEVICE_NAME        "video"
#endif

int xgui_screen_open(xgui_screen_t *screen)
{
    video_info_t video_info;
    int fd = opendev( DEVICE_NAME, 0 );
    if (fd < 0)
        return -1;
    int ret = ioctl(fd, VIDEOIO_GETINFO, (void *) &video_info);
    if (ret < 0) {
        close(fd);
        return -1;
    }
    screen->width = video_info.x_resolution;
    screen->height = video_info.y_resolution;
    screen->bpp = video_info.bits_per_pixel;
#ifdef XGUI_DEBUG
    printf("xgui: width %d, height %d, bpp %d\n", screen->width, screen->height, screen->bpp);
#endif
    unsigned int vram_size = video_info.bytes_per_scan_line * video_info.y_resolution;
    if (!vram_size) {
        close(fd);
        return -1;
    }
#ifdef XGUI_DEBUG
    printf("xgui: screen memory map size %x\n", vram_size);
#endif
    unsigned char *vram = mmap(fd, vram_size, 0);
    if (vram == NULL) {
        printf("xgui: video mapped failed!\n");
        close(fd);
        return -1;
    }
    screen->vram_start = vram;
    screen->handle = fd;
#ifdef XGUI_DEBUG
    printf("xgui: mapped addr %x\n", vram);
#endif
    return 0;
}

int xgui_screen_close(xgui_screen_t *screen)
{
    if (!screen)
        return -1;
    munmap(screen->vram_start, 0);
    close(screen->handle);
    screen->vram_start = NULL;
    screen->handle = -1;
    return 0;
}
