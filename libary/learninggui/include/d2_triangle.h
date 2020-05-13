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

#ifndef   __LGUI_D2_TRIANGLE_HEADER__
#define   __LGUI_D2_TRIANGLE_HEADER__

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


/* Triangle direction */
#ifndef  TRIANGLE_LEFT
#define  TRIANGLE_LEFT      0x01
#endif
#ifndef  TRIANGLE_RIGHT
#define  TRIANGLE_RIGHT     0x02
#endif
#ifndef  TRIANGLE_UP
#define  TRIANGLE_UP        0x03
#endif
#ifndef  TRIANGLE_DOWN
#define  TRIANGLE_DOWN      0x04
#endif


#ifdef  _LG_TRIANGLE_

#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_fill_triangle(HDC hdc, int left, int top, int right, int bottom, unsigned int dir);

    #ifndef  _LG_ALONE_VERSION_
    int  fill_triangle(HDC hdc, int left, int top, int right, int bottom, unsigned int dir);
    #else
    #define  fill_triangle(hdc, left, top, right, bottom, dir)        in_fill_triangle(hdc, left, top, right, bottom, dir)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_TRIANGLE_ */

#endif  /* __LGUI_D2_TRIANGLE_HEADER__ */
