#ifndef _GUI_DRAW_H
#define _GUI_DRAW_H


#include "layer.h"
#include "color.h"

int layer_put_point(layer_t *layer, int x, int y, GUI_COLOR color);
int layer_get_point(layer_t *layer, int x, int y, GUI_COLOR *color);

void layer_put_vline(layer_t *layer, int left, int top, int buttom, GUI_COLOR color);
void layer_put_hline(layer_t *layer, int left, int right, int top, GUI_COLOR color);

void layer_draw_line(layer_t *layer, int x0, int y0, int x1, int y1, GUI_COLOR color);

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

void layer_sync_bitmap(layer_t *layer, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region);

#endif  /* _GUI_DRAW_H */
