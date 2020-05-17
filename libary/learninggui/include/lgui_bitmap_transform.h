/*   
 *  Copyright (C) 2011- 2020 Rao Youkun(960747373@qq.com)
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

#ifndef  __LGUI_BITMAP_TRANSFORM_HEADER__
#define  __LGUI_BITMAP_FRANSFORM_HEADER__        1

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




#ifdef  __cplusplus
extern  "C"
{
#endif


    int  in_is_point_inside_convex_polygon(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT *corner);
    int  in_make_blank_grid(unsigned int width, unsigned int height, void *transform);
    DOUBLE  in_transform_caliculate_overlap(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT  *corner);
    int  in_gui_bitmap_transform_area(HDC hdc, const void *bitmap, void *transform);


    #ifndef  _LG_ALONE_VERSION_

    int  is_point_inside_convex_polygon(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT *corner);
    int  make_blank_grid(unsigned int width, unsigned int height, void *transform);
    DOUBLE  transform_caliculate_overlap(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT  *corner);
    int  gui_bitmap_transform_area(HDC hdc, const void *bitmap, void *transform);

    #else  /* _LG_ALONE_VERSION_ */

    #define  is_point_inside_convex_polygon(p, corner)  in_is_point_inside_convex_polygon(p, corner)
    #define  make_blank_grid(width, height, transform)  in_make_blank_grid(width, height, transform)
    #define  transform_caliculate_overlap(p, corner)  in_transform_caliculate_overlap(p, corner)
    #define  gui_bitmap_transform_area(hdc, bitmap, transform)  in_gui_bitmap_transform_area(hdc,bitmap, transform)


    #endif  /* _LG_ALONE_VERSION_ */


#ifdef  __cplusplus
}
#endif


#endif  /* __LGUI_BITMAP_TRANSFORM_HEADER__ */

