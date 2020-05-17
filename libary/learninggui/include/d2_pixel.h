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

#ifndef  __LGUI_D2_PIXEL_HEADER__
#define  __LGUI_D2_PIXEL_HEADER__

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



#ifdef  _LG_DC_

#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal funciton */
    int  in_no_clip_output_screen_pixel_abs(HDC hdc, int x, int y, SCREEN_COLOR  color);
    int  in_output_screen_pixel_abs(HDC hdc, int x, int y, SCREEN_COLOR  color);
    /* Internal function end */


    int  in_point_output_pixel(HDC hdc, int x, int y);
    int  in_point_input_pixel(HDC hdc, int x, int y, GUI_COLOR  *color);
    int  in_point_input_pixel_abs(HDC hdc, int x, int y, GUI_COLOR  *color);

    #ifndef  _LG_ALONE_VERSION_
    int  point_output_pixel(HDC hdc, int x, int y);
    int  point_input_pixel(HDC hdc, int x, int y, GUI_COLOR  *color);
    int  point_input_pixel_abs(HDC hdc, int x, int y, GUI_COLOR  *color);
    #else
    #define  point_output_pixel(hdc, x, y)               in_point_output_pixel(hdc, x, y)
    #define  point_input_pixel(hdc, x, y, color)         in_point_input_pixel(hdc, x, y, color) 
    #define  point_input_pixel_abs(hdc, x, y, color)     in_point_input_pixel_abs(hdc, x, y, color)
    #endif  /* _LG_ALONE_VERSION_ */


#ifdef  __cplusplus
}
#endif

#endif  /* _LG_DC_ */

#endif  /* __LGUI_D2_PIXEL_HEADER__ */
