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

#ifndef  __LGUI_BITMAP_ROTATE_HEADER__
#define  __LGUI_BITMAP_ROTATE_HEADER__        1

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


    int  in_is_corner_inside_square(GUI_DOUBLE_POINT *point, GUI_DOUBLE_POINT *square, DOUBLE *fsin, DOUBLE *fcos);

    int  in_sort_point(GUI_DOUBLE_POINT *in_point, unsigned int in_size, GUI_DOUBLE_POINT *out_point, unsigned int *out_size);

    DOUBLE  in_caliculate_area(GUI_DOUBLE_POINT *point, unsigned int size);


    DOUBLE  in_caliculate_overlap(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT  *center, DOUBLE *fsin, DOUBLE  *fcos);


    int  in_gui_bitmap_rotate_area(HDC hdc, int x, int y, const void *bitmap, void *rotate);


    #ifndef  _LG_ALONE_VERSION_

    int  is_corner_inside_square(GUI_DOUBLE_POINT *point, GUI_DOUBLE_POINT *square, DOUBLE *fsin, DOUBLE *fcos);

    int  sort_point(GUI_DOUBLE_POINT *in_point, unsigned int in_size, GUI_DOUBLE_POINT *out_point, unsigned int *out_size);

    DOUBLE  caliculate_area(GUI_DOUBLE_POINT *point, unsigned int size);

    DOUBLE  caliculate_overlap(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT  *center, DOUBLE *fsin, DOUBLE *fcos);


    int  gui_bitmap_rotate_area(HDC hdc, int x, int y, const void *bitmap, void *rotate);

    #else  /* _LG_ALONE_VERSION_ */

    #define  is_corner_inside_square(point, squrae, fsin, fcos)   in_is_corner_inside_square(point, squrae, fsin, fcos)
    #define  sort_point(in_point,in_size,out_point,out_size)   in_sort_point(in_point,in_size,out_point, out_size)
    #define  caliculate_area(point, size, fsin, fcos)   in_caliculate_area(point, size, fsin, fcos)


    #define  caliculate_overlap(p, center)  in_caliculate_overlap(p, center)


    #define  gui_bitmap_rotate_area(hdc,x,y,bitmap,rotate)  in_gui_bitmap_rotate_area(hdc,x,y,bitmap,rotate)

    #endif  /* _LG_ALONE_VERSION_ */


#ifdef  __cplusplus
}
#endif


#endif  /* __LGUI_BITMAP_ROTATE_HEADER__ */
