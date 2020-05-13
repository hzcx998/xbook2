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

#ifndef  __LGUI_WIN_CELL_WIDGET_HEADER__
#define  __LGUI_WIN_CELL_WIDGET_HEADER__    1

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



/* Cell window color */
#ifndef  CELL_WIN_DISABLED_BCOLOR
#define  CELL_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  CELL_WIN_DISABLED_FCOLOR
#define  CELL_WIN_DISABLED_FCOLOR              GUI_DARK
#endif

#ifndef  CELL_WIN_INACTIVE_BCOLOR
#define  CELL_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  CELL_WIN_INACTIVE_FCOLOR
#define  CELL_WIN_INACTIVE_FCOLOR              GUI_GRAY
#endif

#ifndef  CELL_WIN_ACTIVE_BCOLOR
#define  CELL_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  CELL_WIN_ACTIVE_FCOLOR
#define  CELL_WIN_ACTIVE_FCOLOR                GUI_BLACK
#endif


/* Cell client color */
#ifndef  CELL_CLI_DISABLED_BCOLOR
#define  CELL_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  CELL_CLI_DISABLED_FCOLOR
#define  CELL_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  CELL_CLI_DISABLED_TBCOLOR
#define  CELL_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  CELL_CLI_DISABLED_TFCOLOR
#define  CELL_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  CELL_CLI_INACTIVE_BCOLOR
#define  CELL_CLI_INACTIVE_BCOLOR              GUI_BLACK
#endif
#ifndef  CELL_CLI_INACTIVE_FCOLOR
#define  CELL_CLI_INACTIVE_FCOLOR              GUI_WHITE
#endif
#ifndef  CELL_CLI_INACTIVE_TBCOLOR
#define  CELL_CLI_INACTIVE_TBCOLOR             GUI_BLACK
#endif
#ifndef  CELL_CLI_INACTIVE_TFCOLOR
#define  CELL_CLI_INACTIVE_TFCOLOR             GUI_WHITE
#endif

#ifndef  CELL_CLI_ACTIVE_BCOLOR
#define  CELL_CLI_ACTIVE_BCOLOR                GUI_BLACK
#endif
#ifndef  CELL_CLI_ACTIVE_FCOLOR
#define  CELL_CLI_ACTIVE_FCOLOR                GUI_WHITE
#endif
#ifndef  CELL_CLI_ACTIVE_TBCOLOR
#define  CELL_CLI_ACTIVE_TBCOLOR               GUI_BLACK
#endif
#ifndef  CELL_CLI_ACTIVE_TFCOLOR
#define  CELL_CLI_ACTIVE_TFCOLOR               GUI_WHITE
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_CELL_WIDGET_

/* GUI_CELL */
struct  _GUI_CELL
{
    BINT  unused;
};
typedef	struct	_GUI_CELL  GUI_CELL;

typedef GUI_CELL    IN_GUI_CELL;

#define  GET_IN_GUI_CELL(hwnd)           ((IN_GUI_CELL *)(((HWND)hwnd)->ext))


#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_cell_create(HWND parent, void *gui_common_widget, void *gui_cell);

    #ifndef  _LG_ALONE_VERSION_
    HWND  cell_create(HWND parent, void *gui_common_widget, void *gui_cell);
    #else
    #define  cell_create(parent, gui_common_widget, gui_cell)      in_cell_create(parent, gui_common_widget, gui_cell)
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_CELL_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_CELL_WIDGET_HEADER__ */
