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

#ifndef  __LGUI_COLOR_MATCH_HEADER__
#define  __LGUI_COLOR_MATCH_HEADER__

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


/* Window default color */
#ifndef  WIN_DISABLED_BCOLOR
#define  WIN_DISABLED_BCOLOR                   GUI_GRAY
#endif
#ifndef  WIN_DISABLED_FCOLOR
#define  WIN_DISABLED_FCOLOR                   0x00606060
#endif
#ifndef  WIN_DISABLED_TBCOLOR
#define  WIN_DISABLED_TBCOLOR                  GUI_GRAY
#endif
#ifndef  WIN_DISABLED_TFCOLOR
#define  WIN_DISABLED_TFCOLOR                  GUI_BLACK
#endif

#ifndef  WIN_INACTIVE_BCOLOR
#define  WIN_INACTIVE_BCOLOR                   GUI_GRAY
#endif
#ifndef  WIN_INACTIVE_FCOLOR
#define  WIN_INACTIVE_FCOLOR                   0x007A96DF
#endif
#ifndef  WIN_INACTIVE_TBCOLOR 
#define  WIN_INACTIVE_TBCOLOR                  GUI_GRAY
#endif
#ifndef  WIN_INACTIVE_TFCOLOR
#define  WIN_INACTIVE_TFCOLOR                  GUI_BLACK
#endif

#ifndef  WIN_ACTIVE_BCOLOR
#define  WIN_ACTIVE_BCOLOR                     GUI_GRAY
#endif
#ifndef  WIN_ACTIVE_FCOLOR
#define  WIN_ACTIVE_FCOLOR                     0x000055E5
#endif
#ifndef  WIN_ACTIVE_TBCOLOR
#define  WIN_ACTIVE_TBCOLOR                    GUI_GRAY
#endif
#ifndef  WIN_ACTIVE_TFCOLOR
#define  WIN_ACTIVE_TFCOLOR                    GUI_BLACK
#endif

/* Client default color */
#ifndef  CLI_DISABLED_BCOLOR
#define  CLI_DISABLED_BCOLOR                   0x00808080
#endif
#ifndef  CLI_DISABLED_FCOLOR
#define  CLI_DISABLED_FCOLOR                   0x00A0A0A0
#endif
#ifndef  CLI_DISABLED_TBCOLOR
#define  CLI_DISABLED_TBCOLOR                  0x00808080
#endif
#ifndef  CLI_DISABLED_TFCOLOR
#define  CLI_DISABLED_TFCOLOR                  GUI_BLACK
#endif

#ifndef  CLI_INACTIVE_BCOLOR
#define  CLI_INACTIVE_BCOLOR                   0x00C0C0C0
#endif
#ifndef  CLI_INACTIVE_FCOLOR
#define  CLI_INACTIVE_FCOLOR                   GUI_BLACK
#endif
#ifndef  CLI_INACTIVE_TBCOLOR
#define  CLI_INACTIVE_TBCOLOR                  0x00C0C0C0
#endif
#ifndef  CLI_INACTIVE_TFCOLOR
#define  CLI_INACTIVE_TFCOLOR                  GUI_BLACK
#endif

#ifndef  CLI_ACTIVE_BCOLOR
#define  CLI_ACTIVE_BCOLOR                     0x00C0C0C0
#endif
#ifndef  CLI_ACTIVE_FCOLOR
#define  CLI_ACTIVE_FCOLOR                     GUI_BLACK
#endif
#ifndef  CLI_ACTIVE_TBCOLOR
#define  CLI_ACTIVE_TBCOLOR                    0x00C0C0C0
#endif
#ifndef  CLI_ACTIVE_TFCOLOR
#define  CLI_ACTIVE_TFCOLOR                    GUI_BLACK
#endif



#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal function */
    #ifdef  _LG_PALETTE_COLOR_CONVERSION_
    SCREEN_COLOR  in_gui_to_palette_index(GUI_COLOR gui_color);
    #endif

    UINT32  COLOR_MATCH_INDEX(GUI_COLOR color0, GUI_COLOR color1);


#ifdef  __cplusplus
}
#endif


#endif  /* __LGUI_COLOR_MATCH_HEADER__ */
