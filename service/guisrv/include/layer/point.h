#ifndef __GUISRV_LAYER_POINT_H__
#define __GUISRV_LAYER_POINT_H__

#include "color.h"
#include "layer.h"

int layer_put_point(layer_t *layer, int x, int y, GUI_COLOR color);
int layer_get_point(layer_t *layer, int x, int y, GUI_COLOR *color);

#define layer_draw_point layer_put_point

#endif  /* __GUISRV_LAYER_POINT_H__ */
