#ifndef _LIB_XGUI_H
#define _LIB_XGUI_H

#include <xgui_render.h>

int xgui_create_view(int x, int y, int width, int height);
int xgui_destroy_view(int handle);
int xgui_move_view(int handle, int x, int y);
int xgui_show_view(int handle);
int xgui_hide_view(int handle);
int xgui_update_view(int handle, int left, int top, int right, int buttom);

#endif  /* _LIB_XGUI_H */