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

#ifndef  __LGUI_GROUP_BOX_WIDGET_HEADER__
#define  __LGUI_GROUP_BOX_WIDGET_HEADER__    1

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



/* GroupBox window color */
#ifndef  GBOX_WIN_DISABLED_BCOLOR
#define  GBOX_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  GBOX_WIN_DISABLED_FCOLOR
#define  GBOX_WIN_DISABLED_FCOLOR             GUI_DARK
#endif

#ifndef  GBOX_WIN_INACTIVE_BCOLOR
#define  GBOX_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  GBOX_WIN_INACTIVE_FCOLOR
#define  GBOX_WIN_INACTIVE_FCOLOR             GUI_GRAY
#endif

#ifndef  GBOX_WIN_ACTIVE_BCOLOR
#define  GBOX_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
#endif
#ifndef  GBOX_WIN_ACTIVE_FCOLOR
#define  GBOX_WIN_ACTIVE_FCOLOR               GUI_BLACK
#endif


/* GroupBox client color */
#ifndef  GBOX_CLI_DISABLED_BCOLOR
#define  GBOX_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  GBOX_CLI_DISABLED_FCOLOR
#define  GBOX_CLI_DISABLED_FCOLOR             GUI_GRAY
#endif
#ifndef  GBOX_CLI_DISABLED_TBCOLOR
#define  GBOX_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
#endif
#ifndef  GBOX_CLI_DISABLED_TFCOLOR
#define  GBOX_CLI_DISABLED_TFCOLOR            GUI_BLACK
#endif

#ifndef  GBOX_CLI_INACTIVE_BCOLOR
#define  GBOX_CLI_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  GBOX_CLI_INACTIVE_FCOLOR
#define  GBOX_CLI_INACTIVE_FCOLOR             GUI_BLACK
#endif
#ifndef  GBOX_CLI_INACTIVE_TBCOLOR
#define  GBOX_CLI_INACTIVE_TBCOLOR            GUI_LIGHT_GRAY
#endif
#ifndef  GBOX_CLI_INACTIVE_TFCOLOR
#define  GBOX_CLI_INACTIVE_TFCOLOR            GUI_BLACK
#endif

#ifndef  GBOX_CLI_ACTIVE_BCOLOR
#define  GBOX_CLI_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
#endif
#ifndef  GBOX_CLI_ACTIVE_FCOLOR
#define  GBOX_CLI_ACTIVE_FCOLOR               GUI_BLACK
#endif
#ifndef  GBOX_CLI_ACTIVE_TBCOLOR
#define  GBOX_CLI_ACTIVE_TBCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  GBOX_CLI_ACTIVE_TFCOLOR
#define  GBOX_CLI_ACTIVE_TFCOLOR              GUI_BLACK
#endif


#ifndef  MAX_GROUP_BOX_TEXT_LEN
#define  MAX_GROUP_BOX_TEXT_LEN               63
#endif
#if  (MAX_GROUP_BOX_TEXT_LEN < 1)
#undef   MAX_GROUP_BOX_TEXT_LEN
#define  MAX_GROUP_BOX_TEXT_LEN               63
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_GROUP_BOX_WIDGET_

/* GUI_GROUP_BOX */
struct  _GUI_GROUP_BOX
{
    /* GroupBox text */	
    TCHAR        text[MAX_GROUP_BOX_TEXT_LEN+1];

    /* GroupBox text len */	
    unsigned int len;

    /* GroupBox left_offset */	
    unsigned int left_offset;
};
typedef	struct	_GUI_GROUP_BOX  GUI_GROUP_BOX;


typedef	 GUI_GROUP_BOX  IN_GUI_GROUP_BOX;

#define  GET_IN_GUI_GROUP_BOX(hwnd)           ((IN_GUI_GROUP_BOX *)(((HWND)hwnd)->ext))


#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_group_box_create(HWND parent, void *gui_common_widget, void *gui_group_box);
    int   in_group_box_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len);
    int   in_group_box_set_text(HWND hwnd, TCHAR *text, unsigned int  text_len);

    #ifndef  _LG_ALONE_VERSION_
    HWND  group_box_create(HWND parent, void *gui_common_widget, void *gui_group_box);
    int   group_box_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len);
    int   group_box_set_text(HWND hwnd, TCHAR *text, unsigned int  text_len);
    #else  /* _LG_ALONE_VERSION_ */
    #define  group_box_create(parent, gui_common_widget, gui_group_box)      in_group_box_create(parent, gui_common_widget, gui_group_box)
    #define  group_box_get_text(hwnd, text, text_len)                        in_group_box_get_text(hwnd, text, text_len)
    #define  group_box_set_text(hwnd, text, text_len)                        in_group_box_set_text(hwnd, text, text_len)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_GROUP_BOX_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_GROUP_BOX_WIDGET_HEADER__ */
