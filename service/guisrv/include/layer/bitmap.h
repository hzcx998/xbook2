#ifndef __GUISRV_LAYER_BITMAP_H__
#define __GUISRV_LAYER_BITMAP_H__

#include "color.h"

void layer_draw_bitmap(
    layer_t *layer,
    int x,
    int y,
    int width,
    int height,
    GUI_COLOR *buffer
);

#endif  /* __GUISRV_LAYER_BITMAP_H__ */
