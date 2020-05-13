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

#ifndef  __LGUI_WIN_INVALIDATE_HEADER__
#define  __LGUI_WIN_INVALIDATE_HEADER__

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

#ifdef  __cplusplus
extern  "C"
{
#endif

     /* Internal function */

     /* Invalindate Window numbers */
     extern  volatile  unsigned int  linvan;


    int  in_win_invalidate_rect_abs(/* HWND hwnd */ void *hwnd, const /* GUI_RECT */ void *rect);

    int  in_win_invalidate_rect(/* HWND hwnd */ void *hwnd, const /* GUI_RECT */ void *rect);
    #define  in_win_invalidate(hwnd)         in_win_invalidate_rect(hwnd, NULL)

    int  in_win_invalidate_area_abs(const /* GUI_RECT */ void *rect);
    int  in_win_invalidate_below_area_abs( /* HWND hwnd */ void *hwnd, const /* GUI_RECT */ void *rect);

    int  in_win_paint_invalidate_recursion(/* HWND hwnd */ void *hwnd);

    /* Internal function end */




    int  in_win_invalidate_window_rect(void *hwnd, const void *rect);

    #ifndef  _LG_ALONE_VERSION_
    int  win_invalidate_window_rect(void *hwnd, const void *rect);
    #else
    #define  win_invalidate_window_rect(hwnd,rect)         in_win_invalidate_window_rect(hwnd, rect)
    #endif

    #define  win_invalidate(hwnd)                          win_invalidate_window_rect(hwnd, NULL)

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_INVALIDATE_HEADER__ */
