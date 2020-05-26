#ifndef __GUISRV_GRAPH_POINT_H__
#define __GUISRV_GRAPH_POINT_H__

#include "color.h"

int graph_put_point(int x, int y, GUI_COLOR color);
int graph_get_point(int x, int y, GUI_COLOR *color);

#endif  /* __GUISRV_GRAPH_POINT_H__ */
