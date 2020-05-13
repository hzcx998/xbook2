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

#ifndef  __LGUI_WIN_PUSH_BUTTON_WIDGET_HEADER__
#define  __LGUI_WIN_PUSH_BUTTON_WIDGET_HEADER__        1

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


/* PushButton window color */
#ifndef  PBTN_WIN_DISABLED_BCOLOR
#define  PBTN_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  PBTN_WIN_DISABLED_FCOLOR
#define  PBTN_WIN_DISABLED_FCOLOR              GUI_DARK
#endif

#ifndef  PBTN_WIN_INACTIVE_BCOLOR
#define  PBTN_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  PBTN_WIN_INACTIVE_FCOLOR
#define  PBTN_WIN_INACTIVE_FCOLOR              GUI_GRAY
#endif

#ifndef  PBTN_WIN_ACTIVE_BCOLOR
#define  PBTN_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  PBTN_WIN_ACTIVE_FCOLOR
#define  PBTN_WIN_ACTIVE_FCOLOR                GUI_BLACK
#endif


/* PushButton client color */
#ifndef  PBTN_CLI_DISABLED_BCOLOR
#define  PBTN_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  PBTN_CLI_DISABLED_FCOLOR
#define  PBTN_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  PBTN_CLI_DISABLED_TBCOLOR
#define  PBTN_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  PBTN_CLI_DISABLED_TFCOLOR
#define  PBTN_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  PBTN_CLI_INACTIVE_BCOLOR
#define  PBTN_CLI_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  PBTN_CLI_INACTIVE_FCOLOR
#define  PBTN_CLI_INACTIVE_FCOLOR              GUI_BLACK
#endif
#ifndef  PBTN_CLI_INACTIVE_TBCOLOR
#define  PBTN_CLI_INACTIVE_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  PBTN_CLI_INACTIVE_TFCOLOR
#define  PBTN_CLI_INACTIVE_TFCOLOR             GUI_BLACK
#endif

#ifndef  PBTN_CLI_ACTIVE_BCOLOR
#define  PBTN_CLI_ACTIVE_BCOLOR                0x00E0E0E0
#endif
#ifndef  PBTN_CLI_ACTIVE_FCOLOR
#define  PBTN_CLI_ACTIVE_FCOLOR                GUI_BLACK
#endif
#ifndef  PBTN_CLI_ACTIVE_TBCOLOR
#define  PBTN_CLI_ACTIVE_TBCOLOR               GUI_LIGHT_GRAY
#endif
#ifndef  PBTN_CLI_ACTIVE_TFCOLOR
#define  PBTN_CLI_ACTIVE_TFCOLOR               GUI_BLACK
#endif


#ifndef  MAX_PUSH_BUTTON_TEXT_LEN
#define  MAX_PUSH_BUTTON_TEXT_LEN             15
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_PUSH_BUTTON_WIDGET_

/* GUI_PUSH_BUTTON */
struct  _GUI_PUSH_BUTTON
{
    /* PushButton text */	
    TCHAR        text[MAX_PUSH_BUTTON_TEXT_LEN+1];

    /* PushButton text len */	
    unsigned int len;

    /* PushButton ghost */	
    BINT is_ghost_flag;
};
typedef	struct	_GUI_PUSH_BUTTON  GUI_PUSH_BUTTON;

typedef  GUI_PUSH_BUTTON    IN_GUI_PUSH_BUTTON;

#define  GET_IN_GUI_PUSH_BUTTON(hwnd)           ((IN_GUI_PUSH_BUTTON *)(((HWND)hwnd)->ext))



#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_push_button_create(HWND parent, void *gui_common_widget, void *gui_push_button);
    int   in_push_button_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len);
    int   in_push_button_set_text(HWND hwnd, TCHAR *text, unsigned int  text_len);
    int   in_push_button_set_ghost(HWND hwnd);
    int   in_push_button_is_ghost(HWND hwnd);

    #ifndef  _LG_ALONE_VERSION_
    HWND  push_button_create(HWND parent, void *gui_common_widget, void *gui_push_button);
    int   push_button_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len);
    int   push_button_set_text(HWND hwnd, TCHAR *text, unsigned int  text_len);
    int   push_button_set_ghost(HWND hwnd);
    int   push_button_is_ghost(HWND hwnd);
    #else  /* _LG_ALONE_VERSION_ */
    #define  push_button_create(parent, gui_common_widget, gui_push_button)   in_push_button_create(parent, gui_common_widget, gui_push_button)
    #define  push_button_get_text(hwnd, text, text_len)        in_push_button_get_text(hwnd, text, text_len)
    #define  push_button_set_text(hwnd, text, text_len)        in_push_button_set_text(hwnd, text, text_len)
    #define  push_button_set_ghost(hwnd)                       in_push_button_set_ghost(hwnd)
    #define  push_button_is_ghost(hwnd)                        in_push_button_is_ghost(hwnd)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_PUSH_BUTTON_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_PUSH_BUTTON_WIDGET_HEADER__ */
