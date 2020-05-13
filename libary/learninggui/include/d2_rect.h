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

#ifndef   __LGUI_D2_RECT_HEADER__
#define   __LGUI_D2_RECT_HEADER__

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


#ifdef  _LG_RECTANGLE_

#ifndef  GUI_3D_UP_BORDER_FCOLOR
#define  GUI_3D_UP_BORDER_FCOLOR          GUI_BLACK
#endif
#ifndef  GUI_3D_UP_MID_FCOLOR
#define  GUI_3D_UP_MID_FCOLOR             GUI_WHITE
#endif
#ifndef  GUI_3D_UP_IN_FCOLOR
#define  GUI_3D_UP_IN_FCOLOR              GUI_DARK
#endif

#ifndef  GUI_3D_DOWN_BORDER_FCOLOR
#define  GUI_3D_DOWN_BORDER_FCOLOR        GUI_BLACK
#endif
#ifndef  GUI_3D_DOWN_MID_FCOLOR
#define  GUI_3D_DOWN_MID_FCOLOR           GUI_GRAY
#endif
#ifndef  GUI_3D_DOWN_IN_FCOLOR
#define  GUI_3D_DOWN_IN_FCOLOR            GUI_WHITE
#endif
#ifndef  GUI_3D_DOWN_IN2_FCOLOR
#define  GUI_3D_DOWN_IN2_FCOLOR           GUI_LIGHT_GRAY
#endif


#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_rect_frame(HDC hdc, int left, int top, int right, int bottom);
    int  in_rect_fill(HDC hdc, int left, int top, int right, int bottom);

    int  in_fill_3d_up_rect(HDC hdc, int left, int top, int right, int bottom);
    int  in_fill_3d_down_rect(HDC hdc, int left, int top, int right, int bottom);

    int  in_paint_check(HDC hdc, int left, int top, int right, int bottom);
    int  in_paint_cross(HDC hdc, int left, int top, int right, int bottom);

    #ifndef  _LG_ALONE_VERSION_
    int  rect_frame(HDC hdc, int left, int top, int right, int bottom);
    int  rect_fill(HDC hdc, int left, int top, int right, int bottom);

    int  frame_3d_up_rect(HDC hdc, int left, int top, int right, int bottom);
    int  frame_3d_down_rect(HDC hdc, int left, int top, int right, int bottom);

    int  paint_check(HDC hdc, int left, int top, int right, int bottom);
    int  paint_cross(HDC hdc, int left, int top, int right, int bottom);
    #else
    #define  rect_frame(hdc, left, top, right, bottom)            in_rect_frame(hdc, left, top, right, bottom)
    #define  rect_fill(hdc, left, top, right, bottom)             in_rect_fill(hdc, left, top, right, bottom)

    #define  frame_3d_up_rect(hdc, left, top, right, bottom)      in_frame_3d_up_rect(hdc, left, top, right, bottom)
    #define  frame_3d_down_rect(hdc, left, top, right, bottom)    in_frame_3d_down_rect(hdc, left, top, right, bottom)

    #define  paint_check(hdc, left, top, right, bottom)           in_paint_check(hdc, left, top, right, bottom)
    #define  paint_cross(hdc, left, top, right, bottom)           in_paint_cross(hdc, left, top, right, bottom)
    #endif  /* _LG_ALONE_VERSION_ */


    #define  rect_frame_ext(hdc, rect)    rect_frame(hdc, ((GUI_RECT*)rect)->left, \
                ((GUI_RECT*)rect)->top, ((GUI_RECT*)rect)->right, ((GUI_RECT*)rect)->bottom )
    #define  rect_fill_ext(hdc, rect)     rect_fill(hdc, ((GUI_RECT*)rect)->left, \
                ((GUI_RECT*)rect)->top, ((GUI_RECT*)rect)->right, ((GUI_RECT*)rect)->bottom )

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_RECTANGLE_ */

#endif  /* __LGUI_D2_RECT_HEADER__ */
