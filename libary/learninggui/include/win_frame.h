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

#ifndef  __LGUI_FRAME_HEADER__
#define  __LGUI_FRAME_HEADER__    1

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


#ifdef  _LG_WINDOW_
#include  <win_type_widget.h>
#endif


/* Frame window color */
#ifndef  FRAME_WIN_DISABLED_BCOLOR
#define  FRAME_WIN_DISABLED_BCOLOR              GUI_GRAY
#endif
#ifndef  FRAME_WIN_DISABLED_FCOLOR
#define  FRAME_WIN_DISABLED_FCOLOR              0x00606060
#endif
#ifndef  FRAME_WIN_DISABLED_TBCOLOR
#define  FRAME_WIN_DISABLED_TBCOLOR             GUI_GRAY
#endif
#ifndef  FRAME_WIN_DISABLED_TFCOLOR
#define  FRAME_WIN_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  FRAME_WIN_INACTIVE_BCOLOR
#define  FRAME_WIN_INACTIVE_BCOLOR              GUI_GRAY
#endif
#ifndef  FRAME_WIN_INACTIVE_FCOLOR
#define  FRAME_WIN_INACTIVE_FCOLOR              0x007A96DF
#endif
#ifndef  FRAME_WIN_INACTIVE_TBCOLOR
#define  FRAME_WIN_INACTIVE_TBCOLOR              GUI_GRAY
#endif
#ifndef  FRAME_WIN_INACTIVE_TFCOLOR
#define  FRAME_WIN_INACTIVE_TFCOLOR              GUI_BLACK
#endif

#ifndef  FRAME_WIN_ACTIVE_BCOLOR
#define  FRAME_WIN_ACTIVE_BCOLOR                GUI_GRAY
#endif
#ifndef  FRAME_WIN_ACTIVE_FCOLOR
#define  FRAME_WIN_ACTIVE_FCOLOR                0x000055E5
#endif

#ifndef  FRAME_WIN_ACTIVE_TBCOLOR
#define  FRAME_WIN_ACTIVE_TBCOLOR               GUI_GRAY
#endif
#ifndef  FRAME_WIN_ACTIVE_TFCOLOR
#define  FRAME_WIN_ACTIVE_TFCOLOR               GUI_BLACK
#endif


/* Frame client color */
#ifndef  FRAME_CLI_DISABLED_BCOLOR
#define  FRAME_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  FRAME_CLI_DISABLED_FCOLOR
#define  FRAME_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  FRAME_CLI_DISABLED_TBCOLOR
#define  FRAME_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  FRAME_CLI_DISABLED_TFCOLOR
#define  FRAME_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  FRAME_CLI_INACTIVE_BCOLOR
#define  FRAME_CLI_INACTIVE_BCOLOR              0x00C0C0C0
#endif
#ifndef  FRAME_CLI_INACTIVE_FCOLOR
#define  FRAME_CLI_INACTIVE_FCOLOR              GUI_BLACK
#endif
#ifndef  FRAME_CLI_INACTIVE_TBCOLOR
#define  FRAME_CLI_INACTIVE_TBCOLOR             0x00C0C0C0
#endif
#ifndef  FRAME_CLI_INACTIVE_TFCOLOR
#define  FRAME_CLI_INACTIVE_TFCOLOR             GUI_BLACK
#endif

#ifndef  FRAME_CLI_ACTIVE_BCOLOR
#define  FRAME_CLI_ACTIVE_BCOLOR                0x00C0C0C0
#endif
#ifndef  FRAME_CLI_ACTIVE_FCOLOR
#define  FRAME_CLI_ACTIVE_FCOLOR                GUI_BLACK
#endif
#ifndef  FRAME_CLI_ACTIVE_TBCOLOR
#define  FRAME_CLI_ACTIVE_TBCOLOR               0x00C0C0C0
#endif
#ifndef  FRAME_CLI_ACTIVE_TFCOLOR
#define  FRAME_CLI_ACTIVE_TFCOLOR               GUI_BLACK
#endif



/*
 * Windows or widgets's structure must be defined inside the macro.
 * Because win_config file maybe not exist.
 */
#ifdef  _LG_WINDOW_
#ifdef  _LG_FRAME_WIDGET_

/* GUI_FRAME */
struct  _GUI_FRAME
{
    /* Window text */	
    TCHAR   text[MAX_WIN_TEXT_LEN+1];

    /* Window text len */	
    unsigned int len;
};
typedef	struct	_GUI_FRAME  GUI_FRAME;


typedef	 GUI_FRAME  IN_GUI_FRAME;

#define  GET_IN_GUI_FRAME(hwnd)           ((IN_GUI_FRAME *)(((HWND)hwnd)->ext))



#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_frame_create(HWND parent, void *gui_common_widget, void *gui_frame);

    #ifndef  _LG_ALONE_VERSION_
    HWND  frame_create(HWND parent, void *gui_common_widget, void *gui_frame);
    #else
    #define  frame_create(parent, gui_common_widget, gui_frame)      in_frame_create(parent, gui_common_widget, gui_frame)
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_FRAME_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_FRAME_HEADER__ */
