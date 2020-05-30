#ifndef __GUISRV_LAYER_LINE_H__
#define __GUISRV_LAYER_LINE_H__

#include "color.h"
#include "layer.h"

void layer_put_vline(layer_t *layer, int left, int top, int buttom, GUI_COLOR color);
void layer_put_hline(layer_t *layer, int left, int right, int top, GUI_COLOR color);

void layer_draw_line(layer_t *layer, int x0, int y0, int x1, int y1, GUI_COLOR color);

#endif  /* __GUISRV_LAYER_LINE_H__ */
