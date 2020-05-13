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

#ifndef  __LGUI_WINDOW_SCBAR_HEADER__
#define  __LGUI_WINDOW_SCBAR_HEADER__

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

#include  <dc.h>


#ifdef  _LG_WINDOW_
#ifdef  _LG_SCROLL_BAR_


#ifndef  SCBAR_X_MULTI_STEP
#define  SCBAR_X_MULTI_STEP                    10
#endif
#if (SCBAR_X_MULTI_STEP < 1)
#undef   SCBAR_X_MULTI_STEP
#define  SCBAR_X_MULTI_STEP                    10
#endif

#ifndef  SCBAR_Y_MULTI_STEP
#define  SCBAR_Y_MULTI_STEP                    10
#endif
#if (SCBAR_Y_MULTI_STEP < 1)
#undef   SCBAR_Y_MULTI_STEP
#define  SCBAR_Y_MULTI_STEP                    10
#endif



#ifndef  SCBAR_HEIGHT_WIDTH
#define  SCBAR_HEIGHT_WIDTH                    20
#endif
#if  SCBAR_HEIGHT_WIDTH < 8
#undef   SCBAR_HEIGHT_WIDTH
#define  SCBAR_HEIGHT_WIDTH                    20
#endif


/* GUI_SCBAR */
struct  _GUI_SCBAR
{
    /* Scroll bar rect */
    GUI_RECT      bar_rect;

    /* First button rect */
    GUI_RECT      fbtn_rect;

    /* Last button rect */
    GUI_RECT      lbtn_rect;

    /* Middle rect */
    GUI_RECT      mid_rect;

    /* Middle rect length */
    unsigned int  mid_len;
};
typedef	struct	_GUI_SCBAR   GUI_SCBAR;



#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal fuction */
    /* Temp scroll bar buffer */
    extern  volatile  GUI_SCBAR   lscbar;

    GUI_SCBAR  *in_get_schbar(/* HWND hwnd */ void *hwnd);
    int  in_paint_schbar(/* HWND hwnd */ void *hwnd);

    GUI_SCBAR  *in_get_scvbar(/* HWND hwnd */ void *hwnd);
    int  in_paint_scvbar(/* HWND hwnd */ void *hwnd);
    /* Internal function end */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_SCROLL_BAR_ */
#endif  /* _LG_WINDOW_ */


#endif  /* __LGUI_WINDOW_SCBAR_HEADER__ */
