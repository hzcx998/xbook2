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

#ifndef  __LGUI_LINE_EDIT_WIDGET_HEADER__
#define  __LGUI_LINE_EDIT_WIDGET_HEADER__    1

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


/* Line Edit window color */
#ifndef  LEDIT_WIN_DISABLED_BCOLOR
#define  LEDIT_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LEDIT_WIN_DISABLED_FCOLOR
#define  LEDIT_WIN_DISABLED_FCOLOR              GUI_DARK
#endif

#ifndef  LEDIT_WIN_INACTIVE_BCOLOR
#define  LEDIT_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LEDIT_WIN_INACTIVE_FCOLOR
#define  LEDIT_WIN_INACTIVE_FCOLOR              GUI_GRAY
#endif

#ifndef  LEDIT_WIN_ACTIVE_BCOLOR
#define  LEDIT_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  LEDIT_WIN_ACTIVE_FCOLOR
#define  LEDIT_WIN_ACTIVE_FCOLOR                GUI_BLACK
#endif

/* Line Edit client color */
#ifndef  LEDIT_CLI_DISABLED_BCOLOR
#define  LEDIT_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LEDIT_CLI_DISABLED_FCOLOR
#define  LEDIT_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  LEDIT_CLI_DISABLED_TBCOLOR
#define  LEDIT_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  LEDIT_CLI_DISABLED_TFCOLOR
#define  LEDIT_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  LEDIT_CLI_INACTIVE_BCOLOR
#define  LEDIT_CLI_INACTIVE_BCOLOR              GUI_LIGHT_WHITE
#endif
#ifndef  LEDIT_CLI_INACTIVE_FCOLOR
#define  LEDIT_CLI_INACTIVE_FCOLOR              GUI_BLACK
#endif
#ifndef  LEDIT_CLI_INACTIVE_TBCOLOR
#define  LEDIT_CLI_INACTIVE_TBCOLOR             GUI_WHITE
#endif
#ifndef  LEDIT_CLI_INACTIVE_TFCOLOR
#define  LEDIT_CLI_INACTIVE_TFCOLOR             GUI_BLACK
#endif

#ifndef  LEDIT_CLI_ACTIVE_BCOLOR
#define  LEDIT_CLI_ACTIVE_BCOLOR                GUI_YELLOW
#endif
#ifndef  LEDIT_CLI_ACTIVE_FCOLOR
#define  LEDIT_CLI_ACTIVE_FCOLOR                GUI_BLACK
#endif
#ifndef  LEDIT_CLI_ACTIVE_TBCOLOR
#define  LEDIT_CLI_ACTIVE_TBCOLOR               GUI_WHITE
#endif
#ifndef  LEDIT_CLI_ACTIVE_TFCOLOR
#define  LEDIT_CLI_ACTIVE_TFCOLOR               GUI_BLACK
#endif

    
#ifndef  MAX_LINE_EDIT_TEXT_LEN
#define  MAX_LINE_EDIT_TEXT_LEN                 255
#endif
#if  (MAX_LINE_EDIT_TEXT_LEN < 1)
#undef   MAX_LINE_EDIT_TEXT_LEN
#define  MAX_LINE_EDIT_TEXT_LEN                 255
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_LINE_EDIT_WIDGET_

/* GUI_LINE_EDIT */
struct  _GUI_LINE_EDIT
{
    /* Edit text */	
    TCHAR         text[MAX_LINE_EDIT_TEXT_LEN+1];

    /* Edit text len */	
    unsigned int  len;
};
typedef	struct	_GUI_LINE_EDIT  GUI_LINE_EDIT;


typedef	GUI_LINE_EDIT    IN_GUI_LINE_EDIT;

#define  GET_IN_GUI_LINE_EDIT(hwnd)           ((IN_GUI_LINE_EDIT *)(((HWND)hwnd)->ext))




#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_line_edit_create(HWND parent, void *gui_common_widget, void *gui_line_edit);
    int   in_line_edit_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len);
    int   in_line_edit_set_text(HWND hwnd, TCHAR *text, unsigned int text_len);

    #ifndef  _LG_ALONE_VERSION_
    HWND  line_edit_create(HWND parent, void *gui_common_widget, void *gui_line_edit);
    int   line_edit_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len);
    int   line_edit_set_text(HWND hwnd, TCHAR *text, unsigned int text_len);
    #else  /* _LG_ALONE_VERSION_ */
    #define  line_edit_create(parent, gui_common_widget, gui_line_edit)      in_line_edit_create(parent, gui_common_widget, gui_line_edit)
    #define  line_edit_get_text(hwnd, text, max_len)                         in_line_edit_get_text(hwnd, text, max_len)
    #define  line_edit_set_text(hwnd, text, text_len)                        in_line_edit_set_text(hwnd, text, text_len)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_LINE_EDIT_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_LINE_EDIT_WIDGET_HEADER__ */
