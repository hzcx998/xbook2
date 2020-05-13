/*   
 *  Copyright (C) 2011- 2018 Rao Youkun(960747373@qq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef  __LGUI_WIN_WIDGET_HEADER__
#define  __LGUI_WIN_WIDGET_HEADER__    1

#include  <type_color.h>

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif

#include  <type_gui.h>



#ifdef  _LG_WINDOW_

#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal function */

    /* Temp common widget buffer */
    extern  volatile  GUI_COMMON_WIDGET   ltcomm;

    int  in_set_hwnd_common1(/* HWND */ void *parent, unsigned int widget_type,  /* GUI_COMMMON_WIDGET */ void *gui_common_widget);
    int  in_set_hwnd_common2(/* GUI_COMMMON_WIDGET */ void *gui_common_widget);
    int  in_set_hwnd_common3(/* GUI_COMMMON_WIDGET */ void *gui_common_widget);
    HWND in_malloc_hwnd_memory(/* GUI_COMMMON_WIDGET */ void *gui_common_widget, unsigned int size);
    int  in_deal_add_hwnd(/* GUI_COMMMON_WIDGET */ void *gui_common_widget, HWND hwnd, BUINT acc_hwnd_flag);


    int  in_paint_widget_back(/* HWND */ void *hwnd);
    int  in_paint_widget_border(/* HWND */ void *hwnd);

    /* For GroupBox, need rect to paint border */
    int  in_paint_3d_up_border(/* HWND */ void *hwnd, /* GUI_RECT */ void *rect);
    int  in_paint_3d_down_border(/* HWND */ void *hwnd, /* GUI_RECT */ void *rect);


    int  in_common_widget_callback(/* GUI_MESSAGE *msg */ void *msg);

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_WIDGET_HEADER__ */
