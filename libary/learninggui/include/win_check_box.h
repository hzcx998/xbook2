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

#ifndef  __LGUI_WIN_CHECK_BOX_WIDGET_HEADER__
#define  __LGUI_WIN_CHECK_BOX_WIDGET_HEADER__        1

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



/* CheckBox window color */
#ifndef  CKBOX_WIN_DISABLED_BCOLOR
#define  CKBOX_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  CKBOX_WIN_DISABLED_FCOLOR
#define  CKBOX_WIN_DISABLED_FCOLOR              GUI_DARK
#endif

#ifndef  CKBOX_WIN_INACTIVE_BCOLOR
#define  CKBOX_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  CKBOX_WIN_INACTIVE_FCOLOR
#define  CKBOX_WIN_INACTIVE_FCOLOR              GUI_GRAY
#endif

#ifndef  CKBOX_WIN_ACTIVE_BCOLOR
#define  CKBOX_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  CKBOX_WIN_ACTIVE_FCOLOR
#define  CKBOX_WIN_ACTIVE_FCOLOR                GUI_BLACK
#endif


/* CheckBox client color */
#ifndef  CKBOX_CLI_DISABLED_BCOLOR
#define  CKBOX_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  CKBOX_CLI_DISABLED_FCOLOR
#define  CKBOX_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  CKBOX_CLI_DISABLED_TBCOLOR
#define  CKBOX_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  CKBOX_CLI_DISABLED_TFCOLOR
#define  CKBOX_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  CKBOX_CLI_INACTIVE_BCOLOR
#define  CKBOX_CLI_INACTIVE_BCOLOR              GUI_LIGHT_WHITE
#endif
#ifndef  CKBOX_CLI_INACTIVE_FCOLOR
#define  CKBOX_CLI_INACTIVE_FCOLOR              GUI_BLACK
#endif
#ifndef  CKBOX_CLI_INACTIVE_TBCOLOR
#define  CKBOX_CLI_INACTIVE_TBCOLOR             GUI_WHITE
#endif
#ifndef  CKBOX_CLI_INACTIVE_TFCOLOR
#define  CKBOX_CLI_INACTIVE_TFCOLOR             GUI_BLACK
#endif

#ifndef  CKBOX_CLI_ACTIVE_BCOLOR
#define  CKBOX_CLI_ACTIVE_BCOLOR                GUI_YELLOW
#endif
#ifndef  CKBOX_CLI_ACTIVE_FCOLOR
#define  CKBOX_CLI_ACTIVE_FCOLOR                GUI_BLACK
#endif
#ifndef  CKBOX_CLI_ACTIVE_TBCOLOR
#define  CKBOX_CLI_ACTIVE_TBCOLOR               GUI_WHITE
#endif
#ifndef  BTN_CLI_ACTIVE_TFCOLOR
#define  BTN_CLI_ACTIVE_TFCOLOR                 GUI_BLACK
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_CHECK_BOX_WIDGET_

/* GUI_CHECK_BOX */
struct  _GUI_CHECK_BOX
{
    /* CheckBox state */	
    unsigned char  state;
};
typedef	struct	_GUI_CHECK_BOX  GUI_CHECK_BOX;


/* IN_GUI_CHECK_BOX */
typedef GUI_CHECK_BOX  IN_GUI_CHECK_BOX;

#define  GET_IN_GUI_CHECK_BOX(hwnd)           ((IN_GUI_CHECK_BOX *)(((HWND)hwnd)->ext))



#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_check_box_create(HWND parent, void *gui_common_widget, void *gui_check_box);
    int   in_check_box_get_state(HWND hwnd);
    int   in_check_box_set_state(HWND hwnd, unsigned char state);

    #ifndef  _LG_ALONE_VERSION_
    HWND  check_box_create(HWND parent, void *gui_common_widget, void *gui_check_box);
    int   check_box_get_state(HWND hwnd);
    int   check_box_set_state(HWND hwnd, unsigned char state);
    #else  /* _LG_ALONE_VERSION_ */
    #define  check_box_create(parent, gui_common_widget, gui_check_box)   in_check_box_create(parent, gui_common_widget, gui_check_box)
    #define  check_box_get_state(hwnd)                                    in_check_box_get_state(hwnd)
    #define  check_box_set_state(hwnd, state)                             in_check_box_set_state(hwnd, state)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_CHECK_BOX_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_CHECK_BOX_WIDGET_HEADER__ */
