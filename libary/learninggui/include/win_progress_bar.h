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

#ifndef  __LGUI_WIN_PROGRESS_BAR_WIDGET_HEADER__
#define  __LGUI_WIN_PROGRESS_BAR_WIDGET_HEADER__        1

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




/* ProgressBar ext style */
#ifndef  PGBAR_HBAR_STYLE
#define  PGBAR_HBAR_STYLE                (1<<0x00)
#endif
#ifndef  PGBAR_VBAR_STYLE
#define  PGBAR_VBAR_STYLE                (1<<0x01)
#endif

#ifndef  PGBAR_STYLE_MASK
#define  PGBAR_STYLE_MASK                (PGBAR_HBAR_STYLE | PGBAR_VBAR_STYLE )
#endif

#ifndef  IS_HBAR_PGBAR
#define  IS_HBAR_PGBAR(w)                ((((GUI_COMMON_WIDGET *)w)->ext_style)&PGBAR_HBAR_STYLE)
#endif
#ifndef  IS_VBAR_PGBAR
#define  IS_VBAR_PGBAR(w)                ((((GUI_COMMON_WIDGET *)w)->ext_style)&PGBAR_VBAR_STYLE)
#endif


/* ProgressBar dislay style */
#ifndef  PGBAR_DISPLAY_PERCENT_STYLE
#define  PGBAR_DISPLAY_PERCENT_STYLE     0x00
#endif
#ifndef  PGBAR_DISPLAY_VALUE_STYLE
#define  PGBAR_DISPLAY_VALUE_STYLE       0x01
#endif


/* ProgressBar window color */
#ifndef  PGBAR_WIN_DISABLED_BCOLOR
#define  PGBAR_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  PGBAR_WIN_DISABLED_FCOLOR
#define  PGBAR_WIN_DISABLED_FCOLOR             GUI_DARK
#endif

#ifndef  PGBAR_WIN_INACTIVE_BCOLOR
#define  PGBAR_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  PGBAR_WIN_INACTIVE_FCOLOR
#define  PGBAR_WIN_INACTIVE_FCOLOR             GUI_GRAY
#endif

#ifndef  PGBAR_WIN_ACTIVE_BCOLOR
#define  PGBAR_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
#endif
#ifndef  PGBAR_WIN_ACTIVE_FCOLOR
#define  PGBAR_WIN_ACTIVE_FCOLOR               GUI_BLACK
#endif


/* ProgressBar client color */
#ifndef  PGBAR_CLI_DISABLED_BCOLOR
#define  PGBAR_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  PGBAR_CLI_DISABLED_FCOLOR
#define  PGBAR_CLI_DISABLED_FCOLOR             GUI_GRAY
#endif
#ifndef  PGBAR_CLI_DISABLED_TBCOLOR
#define  PGBAR_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
#endif
#ifndef  PGBAR_CLI_DISABLED_TFCOLOR
#define  PGBAR_CLI_DISABLED_TFCOLOR            GUI_BLACK
#endif

#ifndef  PGBAR_CLI_INACTIVE_BCOLOR
#define  PGAR_CLI_INACTIVE_BCOLOR              GUI_GRAY
#endif
#ifndef  PGBAR_CLI_INACTIVE_FCOLOR
#define  PGBAR_CLI_INACTIVE_FCOLOR             GUI_BLACK
#endif
#ifndef  PGBAR_CLI_INACTIVE_TBCOLOR
#define  PGBAR_CLI_INACTIVE_TBCOLOR            GUI_GRAY
#endif
#ifndef  PGBAR_CLI_INACTIVE_TFCOLOR
#define  PGBAR_CLI_INACTIVE_TFCOLOR            GUI_BLACK
#endif

#ifndef  PGBAR_CLI_ACTIVE_BCOLOR
#define  PGBAR_CLI_ACTIVE_BCOLOR               GUI_GRAY
#endif
#ifndef  PGBAR_CLI_ACTIVE_FCOLOR
#define  PGBAR_CLI_ACTIVE_FCOLOR               GUI_BLACK
#endif
#ifndef  PGBAR_CLI_ACTIVE_TBCOLOR
#define  PGBAR_CLI_ACTIVE_TBCOLOR              GUI_GRAY
#endif
#ifndef  PGBAR_CLI_ACTIVE_TFCOLOR
#define  PGBAR_CLI_ACTIVE_TFCOLOR              GUI_BLACK
#endif


/* ProgressBar max decimal digits */    
#ifndef  PGBAR_MAX_DECIMAL_DIGITS
#define  PGBAR_MAX_DECIMAL_DIGITS              6
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_PROGRESS_BAR_WIDGET_

/* GUI_PROGRESS_BAR */
struct  _GUI_PROGRESS_BAR
{
    /* Special */
    /* Min value */
    int     min_value;  

    /* Max value */
    int     max_value;  

    /* Current value */
    int     current_value;  

    /* Display style */
    BUINT   is_display_text;  

    /* Display style */
    BUINT   display_style;  

    /* Decimal digits */
    BUINT   decimal_digits;  
};
typedef	struct	_GUI_PROGRESS_BAR  GUI_PROGRESS_BAR;


typedef	GUI_PROGRESS_BAR  IN_GUI_PROGRESS_BAR;

#define  GET_IN_GUI_PROGRESS_BAR(hwnd)           ((IN_GUI_PROGRESS_BAR *)(((HWND)hwnd)->ext))


#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_progress_bar_create(HWND parent, void *gui_common_widget, void *gui_progress_bar);

    int   in_progress_bar_get_value(HWND hwnd, int *value);
    int   in_progress_bar_get_percent(HWND hwnd, unsigned int *percent);

    int   in_progress_bar_set_value(HWND hwnd, int value);
    int   in_progress_bar_set_percent(HWND hwnd, unsigned int percent);


    #ifndef  _LG_ALONE_VERSION_
    HWND  progress_bar_create(HWND parent, void *gui_common_widget, void *gui_progress_bar);

    int   progress_bar_get_value(HWND hwnd, int *value);
    int   progress_bar_get_percent(HWND hwnd, unsigned int *percent);

    int   progress_bar_set_value(HWND hwnd, int value);
    int   progress_bar_set_percent(HWND hwnd, unsigned int percent);

    #else  /* _LG_ALONE_VERSION_ */
    #define  progress_bar_create(parent, gui_common_widget, gui_progress_bar)   in_progress_bar_create(parent, gui_common_widget, gui_progress_bar)

    #define  progress_bar_get_value(hwnd, value)                in_progress_bar_get_value(hwnd, value)
    #define  progress_bar_get_percent(hwnd, percent)            in_progress_bar_get_percent(hwnd, percent)

    #define  progress_bar_set_value(hwnd, value)                in_progress_bar_set_value(hwnd, value)
    #define  progress_bar_set_percent(hwnd, percent)            in_progress_bar_set_percent(hwnd, percent)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_PROGRESS_BAR_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_PROGRESS_BAR_WIDGET_HEADER__ */
