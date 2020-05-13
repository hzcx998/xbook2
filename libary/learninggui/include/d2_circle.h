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

#ifndef  __LGUI_D2_CIRCLE_HEADER__
#define  __LGUI_D2_CIRCLE_HEADER__

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




#ifdef  _LG_CIRCLE_

#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_circle(HDC hdc, int x, int y, unsigned int r);
    int  in_circle_fill(HDC hdc, int x, int y, unsigned int r);

    #ifndef  _LG_ALONE_VERSION_
    int  circle(HDC hdc, int x, int y, unsigned int r);
    int  circle_fill(HDC hdc, int x, int y, unsigned int r);
    #else
    #define  circle(hdc, x, y, r)                in_circle(hdc, x, y, r)
    #define  circle_fill(hdc, x, y, r)           in_circle_fill(hdc, x, y, r)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_CIRCLE_ */

#endif  /* __LGUI_D2_CIRCLE_HEADER__ */
