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

#ifndef  __LGUI_PALETTE_CONVERSION_HEADER__
#define  __LGUI_PALETTE_CONVERSION_HEADER__

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



#ifdef  _LG_COLOR_CONVERSION_
#ifdef  _LG_PALETTE_CONVERSION_

#ifdef  __cplusplus
extern  "C"
{
#endif

    SCREEN_COLOR  in_gui_to_palette_index(GUI_COLOR gui_color);
    GUI_COLOR     in_palette_index_to_gui(SCREEN_COLOR screen_color);

    #ifndef  _LG_ALONE_VERSION_
    SCREEN_COLOR  gui_to_palette_index(GUI_COLOR gui_color);
    GUI_COLOR     palette_index_to_gui(SCREEN_COLOR screen_color);
    #else  /* _LG_ALONE_VERSION_ */
    #define  gui_to_palette_index(gui_color)           in_gui_to_palette_index(gui_color)
    #define  palette_index_to_gui(screen_color)        in_palette_index_to_gui(screen_color)
    #endif  /* _LG_ALONE_VERSION_ */
 
#ifdef  __cplusplus
}
#endif

#endif  /* _LG_PALETTE_CONVERSION_ */
#endif  /* _LG_COLOR_CONVERSION_ */

#endif  /* __LGUI_PALETTE_CONVERSION_HEADER__ */
