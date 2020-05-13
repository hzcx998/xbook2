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

#ifndef  __LGUI_IMAGE_ICON_HEADER__
#define  __LGUI_IMAGE_ICON_HEADER__

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


#include  <palette.h>


#define  ICON_TYPE             0x01
#define  CURSOR_TYPE           0x02


struct  _GUI_ICON
{
          unsigned int    type; 
          unsigned int    width;
          unsigned int    height;
          unsigned int    bits_per_pixel;
          unsigned int    bytes_per_line;
          unsigned int    left;
          unsigned int    top;
          unsigned int    bit16_format;
          unsigned int    is_rle8_format;
          unsigned char   panes_left;
          unsigned char   bpp_top;
    const GUI_PALETTE    *palette;
    const unsigned char  *xor_data;
    const unsigned char  *and_data;
};
typedef  struct  _GUI_ICON   GUI_ICON;


#ifdef  _LG_ICON_

#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_icon_fill_flag(HDC hdc, int x, int y, const void *icon, unsigned int cursor_flag);
    #define  in_icon_fill(hdc, x, y, icon)    in_icon_fill_flag(hdc, x, y, icon, 0)

    #ifdef   _LG_FILL_ICON_EXTENSION_
    int  in_icon_fill_rect(HDC hdc, const void *rect, const void *icon);
    #endif


    #ifndef  _LG_ALONE_VERSION_
    int  icon_fill(HDC hdc, int x, int y, const void *icon);
    #else
    #define  icon_fill(hdc, x, y, icon)       in_icon_fill_flag(hdc, x, y, icon, 0)
    #endif

    #ifdef   _LG_FILL_ICON_EXTENSION_
    #ifndef  _LG_ALONE_VERSION_
    int  icon_fill_rect(HDC hdc, const void *rect, const void *icon);
    #else 
    #define  icon_fill_rect(hdc, rect, icon)   in_icon_fill_rect(hdc, rect, icon)
    #endif
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_ICON_ */

#endif  /* __LGUI_IMAGE_ICON_HEADER__ */
