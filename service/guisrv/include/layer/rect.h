#ifndef __GUISRV_LAYER_RECT_H__
#define __GUISRV_LAYER_RECT_H__

#include "color.h"
#include "layer.h"

void layer_draw_rect_fill(
    layer_t *layer,
    int x,
    int y,
    int width,
    int height,
    GUI_COLOR color
);

void layer_draw_rect(
    layer_t *layer,
    int x,
    int y,
    int width,
    int height,
    GUI_COLOR color
);

#endif  /* __GUISRV_LAYER_RECT_H__ */
