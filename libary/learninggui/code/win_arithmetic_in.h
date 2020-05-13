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

#ifndef  __LGUI_WIN_ARITHMETIC_IN_HEADER__
#define  __LGUI_WIN_ARITHMETIC_IN_HEADER__

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

    /* Global the hwnd list */
    extern  volatile HWND   lhlist;

    /* Global the focus hwnd */
    extern  volatile HWND   lhfocu;

    /* Global the default hwnd */
    extern  volatile  HWND  lhdefa;

    /* Global the MTJT hwnd */
    #ifdef   _LG_MTJT_
    extern  volatile HWND   lhmtjt;
    #endif


    /* Temp GUI_DC buffer */
    extern  volatile  GUI_DC           ltdc;

    /* Temp head buffer */
    extern  volatile  GUI_WND_HEAD     lthead;

    int  in_win_has(/* HWND hwnd*/ void *hwnd);
    int  in_win_add(/* HWND hwnd*/ void *hwnd);
    int  in_win_delete_hide_comm( /* HWND hwnd */ void *hwnd);
    int  in_win_delete( /* HWND hwnd */ void *hwnd);

    int  in_win_arithmetic_init(void);

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_ARITHMETIC_IN_HEADER__ */
