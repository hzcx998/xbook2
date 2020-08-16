#ifndef _GUI_DRAW_H
#define _GUI_DRAW_H


#include "layer.h"
#include "color.h"
void layer_draw_pixmap(layer_t *layer, int x, int y, uint32_t width, uint32_t height, GUI_COLOR *buffer, int bps);

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

#endif  /* _GUI_DRAW_H */
