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

#ifndef  __LGUI_WIN_LABEL_WIDGET_HEADER__
#define  __LGUI_WIN_LABEL_WIDGET_HEADER__    1

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

#include  <message.h>


/* Label window color */
#ifndef  LBL_WIN_DISABLED_BCOLOR
#define  LBL_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LBL_WIN_DISABLED_FCOLOR
#define  LBL_WIN_DISABLED_FCOLOR              GUI_DARK
#endif

#ifndef  LBL_WIN_INACTIVE_BCOLOR
#define  LBL_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LBL_WIN_INACTIVE_FCOLOR
#define  LBL_WIN_INACTIVE_FCOLOR              GUI_GRAY
#endif

#ifndef  LBL_WIN_ACTIVE_BCOLOR
#define  LBL_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  LBL_WIN_ACTIVE_FCOLOR
#define  LBL_WIN_ACTIVE_FCOLOR                GUI_BLACK
#endif


/* Label client color */
#ifndef  LBL_CLI_DISABLED_BCOLOR
#define  LBL_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LBL_CLI_DISABLED_FCOLOR
#define  LBL_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  LBL_CLI_DISABLED_TBCOLOR
#define  LBL_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  LBL_CLI_DISABLED_TFCOLOR
#define  LBL_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  LBL_CLI_INACTIVE_BCOLOR
#define  LBL_CLI_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LBL_CLI_INACTIVE_FCOLOR
#define  LBL_CLI_INACTIVE_FCOLOR              GUI_BLACK
#endif
#ifndef  LBL_CLI_INACTIVE_TBCOLOR
#define  LBL_CLI_INACTIVE_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  LBL_CLI_INACTIVE_TFCOLOR
#define  LBL_CLI_INACTIVE_TFCOLOR             GUI_BLACK
#endif

#ifndef  LBL_CLI_ACTIVE_BCOLOR
#define  LBL_CLI_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  LBL_CLI_ACTIVE_FCOLOR
#define  LBL_CLI_ACTIVE_FCOLOR                GUI_BLACK
#endif
#ifndef  LBL_CLI_ACTIVE_TBCOLOR
#define  LBL_CLI_ACTIVE_TBCOLOR               GUI_LIGHT_GRAY
#endif
#ifndef  LBL_CLI_ACTIVE_TFCOLOR
#define  LBL_CLI_ACTIVE_TFCOLOR               GUI_BLACK
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_LABEL_WIDGET_

/* GUI_LABEL */
struct  _GUI_LABEL
{
    /* Label text */	
    TCHAR        text[MAX_LABEL_TEXT_LEN+1];

    /* Label text */	
    unsigned int len;

    /* No back flag */
    BINT          no_back_flag;
};
typedef	struct	_GUI_LABEL  GUI_LABEL;


typedef  GUI_LABEL    IN_GUI_LABEL;

#define  GET_IN_GUI_LABEL(hwnd)           ((IN_GUI_LABEL *)(((HWND)hwnd)->ext))


#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_label_create(HWND parent, void *gui_common_widget, void *gui_label);
    int   in_label_set_text(HWND hwnd, TCHAR *text, int text_len);
    int   in_label_get_text(HWND hwnd, TCHAR *text, int *text_len);

    #ifndef  _LG_ALONE_VERSION_
    HWND  label_create(HWND parent, void *gui_common_widget, void *gui_label);
    int   label_set_text(HWND hwnd, TCHAR *text, int text_len);
    int   label_get_text(HWND hwnd, TCHAR *text, int *text_len);
    #else  /* _LG_ALONE_VERSION_ */
    #define  label_create(parent, gui_common_widget, gui_label)   in_label_create(parent, gui_common_widget, gui_label)
    #define  label_set_text(hwnd, text, text_len)         in_label_set_text(hwnd, text, text_len)
    #define  label_get_text(hwnd, text, text_len)         in_label_get_text(hwnd, text, text_len)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_LABEL_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_LABEL_WIDGET_HEADER__ */
