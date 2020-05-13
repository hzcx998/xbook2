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

#ifndef  __LGUI_BRUSH_HEADER__
#define  __LGUI_BRUSH_HEADER__        1

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


#ifdef  _LG_BRUSH_

#ifdef  __cplusplus
extern  "C"
{
#endif

    GUI_BRUSH  *in_get_brush(HDC hdc);
    int         in_set_brush(HDC hdc, void *brush);

    #ifndef  _LG_ALONE_VERSION_
    GUI_BRUSH  *get_brush(HDC hdc);
    int         set_brush(HDC hdc, void *brush);
    #else  /* _LG_ALONE_VERSION_ */
    #define     get_brush(hdc)                  in_get_brush(hdc)
    #define     set_brush(hdc,brush)            in_set_brush(hdc,brush)
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_BRUSH_ */

#endif  /*__LGUI_BRUSH_HEADER__*/
