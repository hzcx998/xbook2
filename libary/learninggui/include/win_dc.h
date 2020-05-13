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

#ifndef  __LGUI_WIN_DC_HEADER__
#define  __LGUI_WIN_DC_HEADER__

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

    /* HDC function */
    HDC  in_hdc_get_window(void *hwnd);
    HDC  in_hdc_get_client(void *hwnd);
    int  in_hdc_release_win(void *hwnd, HDC hdc);


    #ifndef  _LG_ALONE_VERSION_

    HDC  hdc_get_window(void *hwnd);
    HDC  hdc_get_client(void *hwnd);
    int  hdc_release_win(void *hwnd, HDC hdc);

    #else  /* _LG_ALONE_VERSION_ */

    #define  hdc_get_window(hwnd)                          in_hdc_get_window(hwnd)
    #define  hdc_get_client(hwnd)                          in_hdc_get_client(hwnd)
    #define  hdc_release_win(hwnd, hdc)                    in_hdc_release_win(hwnd, hdc)

    #endif  /* _LG_ALONE_VERSION_ */


    #define  hdc_release_window(hwnd, hdc)                 hdc_release_win(hwnd, hdc)
    #define  hdc_release_client(hwnd, hdc)                 hdc_release_win(hwnd, hdc)

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_DC_HEADER__ */
