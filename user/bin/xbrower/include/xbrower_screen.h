#ifndef _XGUI_SCREEN_H
#define _XGUI_SCREEN_H

#include <xbrower_color.h>
#include "xbrower_view.h"
#include "xbrower_misc.h"

typedef struct {
    int width;  
    int height;
    int bpp;        /* bits per pixel */
    unsigned char  *vram_start;
    int handle;
    int (*out_pixel)(int x, int y, xbrower_color_t color);
} xbrower_screen_t;

extern xbrower_screen_t xbrower_screen;

void xbrower_screen_write_pixel(int x, int y, xbrower_color_t color);

int xbrower_screen_init();
int xbrower_screen_exit();

#endif /* _XGUI_SCREEN_H */
