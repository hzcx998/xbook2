#ifndef __GUISRV_WINDOW_RECT_H__
#define __GUISRV_WINDOW_RECT_H__

#include "window.h"
int gui_window_draw_rect(gui_window_t *window, int x, int y, int width, int height, GUI_COLOR color);
int gui_window_draw_rect_fill(gui_window_t *window, int x, int y, int width, int height, GUI_COLOR color);

#endif  /* __GUISRV_WINDOW_RECT_H__ */
