#ifndef _GAPI_SCREEN_H
#define _GAPI_SCREEN_H

#include "gshape.h"

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int bits_per_pixel;
    g_region_t window_region;       /* 窗口区域，窗口可以活动的区域 */
} g_screen_t;


extern g_screen_t _g_screen;

#define g_screen _g_screen

int g_get_screen(g_screen_t *screen);
int g_set_screen_window_region(g_region_t *region);

#endif /* _GAPI_SCREEN_H */