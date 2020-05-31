#ifndef __GUISRV_WINDOW_POINT_H__
#define __GUISRV_WINDOW_POINT_H__

#include "window.h"

int gui_window_put_point(gui_window_t *window, int x, int y, GUI_COLOR color);
int gui_window_get_point(gui_window_t *window, int x, int y, GUI_COLOR *color);

#define gui_window_draw_point gui_window_put_point


#endif  /* __GUISRV_WINDOW_POINT_H__ */
