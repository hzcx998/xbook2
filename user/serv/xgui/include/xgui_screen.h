#ifndef _XGUI_SCREEN_H
#define _XGUI_SCREEN_H

#include "xgui_color.h"
#include "xgui_view.h"
#include "xgui_misc.h"

typedef struct {
    int width;  
    int height;
    int bpp;        /* bits per pixel */
    unsigned char  *vram_start;
    int handle;
    int (*out_pixel)(int x, int y, xgui_color_t color);
} xgui_screen_t;

int xgui_screen_init();
void xgui_screen_write_pixel(int x, int y, xgui_color_t color);
void xgui_screen_write_bitmap(int x, int y, xgui_bitmap_t *bitmap);
int xgui_screen_get_width();
int xgui_screen_get_height();

#endif /* _XGUI_SCREEN_H */
