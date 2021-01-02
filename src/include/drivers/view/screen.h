#ifndef _XBOOK_DRIVERS_VIEW_SCREEN_H
#define _XBOOK_DRIVERS_VIEW_SCREEN_H

#include <drivers/view/color.h>
#include "drivers/view/view.h"
#include "drivers/view/misc.h"

typedef struct {
    int width;  
    int height;
    int bpp;        /* bits per pixel */
    unsigned char  *vram_start;
    int handle;
    int (*out_pixel)(int x, int y, view_color_t color);
} view_screen_t;

extern view_screen_t view_screen;

void view_screen_write_pixel(int x, int y, view_color_t color);

int view_screen_init();
int view_screen_exit();

#endif /* _XBOOK_DRIVERS_VIEW_SCREEN_H */
