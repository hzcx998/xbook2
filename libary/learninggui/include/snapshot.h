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

#ifndef  __LGUI_SNAPSHOT_HEADER__
#define  __LGUI_SNAPSHOT_HEADER__

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


#ifdef  _LG_SCREEN_
#ifdef  _LG_SNAPSHOT_
#ifdef  _LG_FILE_SYSTEM_


#ifdef  _LG_WINDOW_
#include  <win_type_widget.h>
#endif


#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_snapshot_set_rect(GUI_RECT *rect);
    #ifdef  _LG_WINDOW_
    int  in_win_snapshot_set_hwnd(void *hwnd);
    #endif
    int  in_snapshot_get(void);
    int  in_snapshot_set_path(const char *path);


    #ifndef  _LG_ALONE_VERSION_

    int  snapshot_set_rect(GUI_RECT *rect);
    #ifdef  _LG_WINDOW_
    int  win_snapshot_set_hwnd(void *hwnd);
    #endif
    int  snapshot_get(void);
    int  snapshot_set_path(const char *path);

    #else

    #define  snapshot_set_rect(rect)         in_snapshot_set_rect(rect)
    #ifdef  _LG_WINDOW_
    #define  win_snapshot_set_hwnd(hwnd)     in_win_snapshot_set_hwnd(hwnd)
    #endif
    #define  snapshot_get()                  in_snapshot_get()
    #define  snapshot_set_path(path)         in_snapshot_set_path(path)

    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_FILE_SYSTEM_ */
#endif  /* _LG_SNAPSHOT_ */
#endif  /* _LG_SCREEN_ */

#endif  /* __LGUI_SNAPSHOT_HEADER__ */
