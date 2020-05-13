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

#ifndef  __LGUI_PEN_HEADER__
#define  __LGUI_PEN_HEADER__

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


#ifdef  _LG_PEN_

#ifdef  __cplusplus
extern  "C"
{
#endif

    GUI_PEN   *in_get_pen(HDC hdc);
    int        in_set_pen(HDC hdc, void *pen);

    #ifndef  _LG_ALONE_VERSION_
    GUI_PEN   *get_pen(HDC hdc);
    int        set_pen(HDC hdc, void *pen);
    #else
    #define    get_pen(hdc)                       in_get_pen(hdc)
    #define    set_pen(hdc,pen)                   in_set_pen(hdc,pen)
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_PEN_ */

#endif  /*__LGUI_PEN_HEADER__*/
