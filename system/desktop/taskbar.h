#ifndef _TASKBAR_H
#define _TASKBAR_H

#include <stdint.h>
#include <gapi.h>

#define TASKBAR_HEIGHT 48

#define TASKBAR_COLOR GC_RGB(128, 128, 128)

typedef struct {
    g_layer_t layer;
    uint32_t width;
    uint32_t height;
    g_color_t color;
    list_t touch_list;
    g_bitmap_t *render;
} taskbar_t;

extern taskbar_t taskbar;

#endif  /* _TASKBAR_H */