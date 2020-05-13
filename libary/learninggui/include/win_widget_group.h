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

#ifndef  __LGUI_WIN_WIDGET_GROUP_HEADER__
#define  __LGUI_WIN_WIDGET_GROUP_HEADER__        1

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
#include  <win_type_widget.h>
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_WIDGET_GROUP_


struct  _IN_GUI_WIDGET_GROUP
{
    HWND  checked_hwnd;
    BINT  checked_flag;
};
typedef	struct	_IN_GUI_WIDGET_GROUP  IN_GUI_WIDGET_GROUP;

#define  GET_IN_GUI_WIDGET_GROUP(hwnd)           ((IN_GUI_WIDGET_GROUP *)(((HWND)hwnd)->ext))



#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_widget_group_create(HWND parent);
    int   in_widget_group_close(HWND hwnd);

    int   in_attach_widget(HWND hwnd, HWND widget_group);
    int   in_detack_widget(HWND hwnd);

    #ifndef  _LG_ALONE_VERSION_
    HWND  widget_group_create(HWND parent);
    int   widget_group_close(HWND hwnd);

    int   attach_widget(HWND hwnd, HWND widget_group);
    int   detack_widget(HWND hwnd);
    #else  /* _LG_ALONE_VERSION_ */
    #define  widget_group_create(parent)                   in_widget_group_create(parent)
    #define  widget_group_close(widget_group)              in_widget_group_close(widget_group)

    #define  attach_widget(hwnd, widget_group)             in_attach_widget(hwnd, widget_group)
    #define  detach_widget(hwnd)                           in_detach_widget(hwnd)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_WIDGET_GROUP_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_WIDGET_GROUP_HEADER__ */
