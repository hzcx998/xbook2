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

#ifndef  __LGUI_WIN_TOOLS_HEADER__
#define  __LGUI_WIN_TOOLS_HEADER__    1

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

    int  in_win_message_post_ext(/* HWND hwnd */ void *hwnd, UINT message_id, int flag);
    int  in_win_message_send_ext(/* HWND hwnd */ void *hwnd, UINT message_id, int flag);
    int  in_make_callback_message(/* HWND hwnd */ void *hwnd, UINT message_id, int flag);

    #ifndef  _LG_ALONE_VERSION_
    int  win_message_post_ext(/* HWND hwnd */ void *hwnd, UINT message_id, int flag);
    int  win_message_send_ext(/* HWND hwnd */ void *hwnd, UINT message_id, int flag);
    int  make_callback_message(/* HWND hwnd */ void *hwnd, UINT message_id, int flag);
    #else  /* _LG_ALONE_VERSION_ */
    #define  win_message_post_ext(hwnd, message_id, flag)        in_win_message_post_ext(hwnd, message_id, flag)
    #define  win_message_send_ext(hwnd, message_id, flag)        in_win_message_send_ext(hwnd, message_id, flag)
    #define  make_callback_message(hwnd, message_id, flag)       in_make_callback_message(hwnd, message_id, flag)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_TOOLS_HEADER__ */
