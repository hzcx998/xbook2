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

#ifndef  __LGUI_WIN_RADIO_BUTTON_WIDGET_HEADER__
#define  __LGUI_WIN_RADIO_BUTTON_WIDGET_HEADER__        1

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


/* RadioButton window color */
#ifndef  RBTN_WIN_DISABLED_BCOLOR
#define  RBTN_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  RBTN_WIN_DISABLED_FCOLOR
#define  RBTN_WIN_DISABLED_FCOLOR              GUI_DARK
#endif

#ifndef  RBTN_WIN_INACTIVE_BCOLOR
#define  RBTN_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  RBTN_WIN_INACTIVE_FCOLOR
#define  RBTN_WIN_INACTIVE_FCOLOR              GUI_GRAY
#endif

#ifndef  RBTN_WIN_ACTIVE_BCOLOR
#define  RBTN_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  RBTN_WIN_ACTIVE_FCOLOR
#define  RBTN_WIN_ACTIVE_FCOLOR                GUI_BLACK
#endif


/* RadioButton client color */
#ifndef  RBTN_CLI_DISABLED_BCOLOR
#define  RBTN_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  RBTN_CLI_DISABLED_FCOLOR
#define  RBTN_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  RBTN_CLI_DISABLED_TBCOLOR
#define  RBTN_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  RBTN_CLI_DISABLED_TFCOLOR
#define  RBTN_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  RBTN_CLI_INACTIVE_BCOLOR
#define  RBTN_CLI_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  RBTN_CLI_INACTIVE_FCOLOR
#define  RBTN_CLI_INACTIVE_FCOLOR              GUI_BLACK
#endif
#ifndef  RBTN_CLI_INACTIVE_TBCOLOR
#define  RBTN_CLI_INACTIVE_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  RBTN_CLI_INACTIVE_TFCOLOR
#define  RBTN_CLI_INACTIVE_TFCOLOR             GUI_BLACK
#endif

#ifndef  RBTN_CLI_ACTIVE_BCOLOR
#define  RBTN_CLI_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  RBTN_CLI_ACTIVE_FCOLOR
#define  RBTN_CLI_ACTIVE_FCOLOR                GUI_BLACK
#endif
#ifndef  RBTN_CLI_ACTIVE_TBCOLOR
#define  RBTN_CLI_ACTIVE_TBCOLOR               GUI_LIGHT_GRAY
#endif
#ifndef  RBTN_CLI_ACTIVE_TFCOLOR
#define  RBTN_CLI_ACTIVE_TFCOLOR               GUI_BLACK
#endif


/* RadioButton delta radius */
#ifndef  RBTN_MINI_RADIUS_OFFSET
#define  RBTN_MINI_RADIUS_OFFSET               2
#endif
#ifndef  RBTN_RADIUS_OFFSET
#define  RBTN_RADIUS_OFFSET                    4
#endif
#if      (RBTN_RADIUS_OFFSET < RBTN_MINI_RADIUS_OFFSET)
#undef   RBTN_RADIUS_OFFSET
#define  RBTN_RADIUS_OFFSET                    RBTN_MINI_RADIUS_OFFSET
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_RADIO_BUTTON_WIDGET_

/* GUI_RADIO_BUTTON */
struct  _GUI_RADIO_BUTTON
{
    BINT  radius_offset;
};
typedef	struct	_GUI_RADIO_BUTTON  GUI_RADIO_BUTTON;


/* IN_GUI_RADIO_BUTTON */
struct  _IN_GUI_RADIO_BUTTON
{
    HWND  group_hwnd;
    BINT  state;
    BINT  connected_flag;
    BINT  radius_offset;
};
typedef	struct	_IN_GUI_RADIO_BUTTON  IN_GUI_RADIO_BUTTON;

#define  GET_IN_GUI_RADIO_BUTTON(hwnd)           ((IN_GUI_RADIO_BUTTON *)(((HWND)hwnd)->ext))



#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_radio_button_create(HWND parent, void *gui_common_widget, void *gui_radio_button);
    int   in_radio_button_get_state(HWND hwnd);
    int   in_radio_button_set_state(HWND hwnd, unsigned char state);

    #ifndef  _LG_ALONE_VERSION_
    HWND  radio_button_create(HWND parent, void *gui_common_widget, void *gui_radio_button);
    int   radio_button_get_state(HWND hwnd);
    int   radio_button_set_state(HWND hwnd, unsigned char state);
    #else  /* _LG_ALONE_VERSION_ */
    #define  radio_button_create(parent, gui_common_widget, gui_radio_button)       in_radio_button_create(parent, gui_common_widget, gui_radio_button)
    #define  radio_button_get_state(hwnd)                       in_radio_button_get_state(hwnd)
    #define  radio_button_set_state(hwnd, state)                in_radio_button_set_state(hwnd, state)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_RADIO_RADIO_BUTTON_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_RADIO_BUTTON_WIDGET_HEADER__ */
