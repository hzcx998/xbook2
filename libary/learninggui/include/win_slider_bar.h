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

#ifndef  __LGUI_SLIDER_BAR_WIDGET_HEADER__
#define  __LGUI_SLIDER_BAR_WIDGET_HEADER__        1

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


/* SliderBar ext style */
#ifndef  SLBAR_HBAR_STYLE
#define  SLBAR_HBAR_STYLE                      (1<<0x00)
#endif
#ifndef  SLBAR_VBAR_STYLE
#define  SLBAR_VBAR_STYLE                      (1<<0x01)
#endif

#ifndef  SLBAR_STYLE_MASK
#define  SLBAR_STYLE_MASK                      (SLBAR_HBAR_STYLE | SLBAR_VBAR_STYLE )
#endif

#ifndef  IS_HBAR_SLBAR
#define  IS_HBAR_SLBAR(w)                      ((((GUI_COMMON_WIDGET *)w)->ext_style)&SLBAR_HBAR_STYLE)
#endif
#ifndef  IS_VBAR_SLBAR
#define  IS_VBAR_SLBAR(w)                      ((((GUI_COMMON_WIDGET *)w)->ext_style)&SLBAR_VBAR_STYLE)
#endif


/* SliderBar dislay style */
#ifndef  SLBAR_DISPLAY_PERCENT_STYLE
#define  SLBAR_DISPLAY_PERCENT_STYLE           0x00
#endif
#ifndef  SLBAR_DISPLAY_VALUE_STYLE
#define  SLBAR_DISPLAY_VALUE_STYLE             0x01
#endif


/* SliderBar window color */
#ifndef  SLBAR_WIN_DISABLED_BCOLOR
#define  SLBAR_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  SLBAR_WIN_DISABLED_FCOLOR
#define  SLBAR_WIN_DISABLED_FCOLOR             GUI_DARK
#endif

#ifndef  SLBAR_WIN_INACTIVE_BCOLOR
#define  SLBAR_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  SLBAR_WIN_INACTIVE_FCOLOR
#define  SLBAR_WIN_INACTIVE_FCOLOR             GUI_GRAY
#endif

#ifndef  SLBAR_WIN_ACTIVE_BCOLOR
#define  SLBAR_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
#endif
#ifndef  SLBAR_WIN_ACTIVE_FCOLOR
#define  SLBAR_WIN_ACTIVE_FCOLOR               GUI_BLACK
#endif


/* SliderBar client color */
#ifndef  SLBAR_CLI_DISABLED_BCOLOR
#define  SLBAR_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  SLBAR_CLI_DISABLED_FCOLOR
#define  SLBAR_CLI_DISABLED_FCOLOR             GUI_GRAY
#endif
#ifndef  SLBAR_CLI_DISABLED_TBCOLOR
#define  SLBAR_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
#endif
#ifndef  SLBAR_CLI_DISABLED_TFCOLOR
#define  SLBAR_CLI_DISABLED_TFCOLOR            GUI_BLACK
#endif

#ifndef  SLBAR_CLI_INACTIVE_BCOLOR
#define  SLBAR_CLI_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  SLBAR_CLI_INACTIVE_FCOLOR
#define  SLBAR_CLI_INACTIVE_FCOLOR             GUI_BLACK
#endif
#ifndef  SLBAR_CLI_INACTIVE_TBCOLOR
#define  SLBAR_CLI_INACTIVE_TBCOLOR            GUI_LIGHT_GRAY
#endif
#ifndef  SLBAR_CLI_INACTIVE_TFCOLOR
#define  SLBAR_CLI_INACTIVE_TFCOLOR            GUI_BLACK
#endif

#ifndef  SLBAR_CLI_ACTIVE_BCOLOR
#define  SLBAR_CLI_ACTIVE_BCOLOR               0x00E0E0E0
#endif
#ifndef  SLBAR_CLI_ACTIVE_FCOLOR
#define  SLBAR_CLI_ACTIVE_FCOLOR               GUI_BLACK
#endif
#ifndef  SLBAR_CLI_ACTIVE_TBCOLOR
#define  SLBAR_CLI_ACTIVE_TBCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  SLBAR_CLI_ACTIVE_TFCOLOR
#define  SLBAR_CLI_ACTIVE_TFCOLOR              GUI_BLACK
#endif


/* SliderBar property */
#ifndef  SLBAR_RULER_HEIGHT
#define  SLBAR_RULER_HEIGHT                    3
#endif
#if  (SLBAR_RULER_HEIGHT < 3)
#undef   SLBAR_RULER_HEIGHT
#define  SLBAR_RULER_HEIGHT                    3
#endif

#ifndef  SLBAR_SLOT_HEIGHT
#define  SLBAR_SLOT_HEIGHT                     6
#endif
#if  (SLBAR_SLOT_HEIGHT < 6)
#undef   SLBAR_SLOT_HEIGHT
#define  SLBAR_SLOT_HEIGHT                     6
#endif

#ifndef  SLBAR_TICK_WIDTH
#define  SLBAR_TICK_WIDTH                      8
#endif
#if  (SLBAR_TICK_WIDTH < 8)
#undef   SLBAR_TICK_WIDTH
#define  SLBAR_TICK_WIDTH                      8
#endif



#ifdef  _LG_WINDOW_
#ifdef  _LG_SLIDER_BAR_WIDGET_

/* GUI_SLIDER_BAR */
struct  _GUI_SLIDER_BAR
{
    /* Min value */
             int  min_value;  

    /* Max value */
             int  max_value;  

    /* Current value */
             int  current_value;  

    /* Step value */
    unsigned int  step_value;  

    /* Decimal digits */
    BUINT   decimal_digits;  

    /* Ruler height */
    BUINT   ruler_height;  

    /* Snap slot height */
    BUINT   slot_height;  

    /* Tick width */
    BUINT   tick_width;  
};
typedef	struct	_GUI_SLIDER_BAR  GUI_SLIDER_BAR;


/* IN_GUI_SLIDER_BAR */
struct  _IN_GUI_SLIDER_BAR
{
    /* Min value */
    int     min_value;  

    /* Max value */
    int     max_value;  

    /* Current value */
    int     current_value;  

    /* Step value */
    unsigned int  step_value;  

    /* Decimal digits */
    BUINT   decimal_digits;  

    /* Ruler height */
    BUINT   ruler_height;  

    /* Snap slot height */
    BUINT   slot_height;  

    /* Tick width */
    BUINT   tick_width;  

    /*  slider rect */
    GUI_RECT   slider_rect;

    /*  tick rect */
    GUI_RECT   tick_rect;
};
typedef	struct	_IN_GUI_SLIDER_BAR  IN_GUI_SLIDER_BAR;

#define  GET_IN_GUI_SLIDER_BAR(hwnd)           ((IN_GUI_SLIDER_BAR *)(((HWND)hwnd)->ext))



#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_slider_bar_create(HWND parent, void *gui_common_widget, void *gui_slider_bar);

    int   in_slider_bar_get_value(HWND hwnd, int *value);
    int   in_slider_bar_get_percent(HWND hwnd, unsigned int *percent);

    int   in_slider_bar_set_value(HWND hwnd, int value);
    int   in_slider_bar_set_percent(HWND hwnd, unsigned int percent);


    #ifndef  _LG_ALONE_VERSION_
    HWND  slider_bar_create(HWND parent, void *gui_common_widget, void *gui_slider_bar);

    int   slider_bar_get_value(HWND hwnd, int *value);
    int   slider_bar_get_percent(HWND hwnd, unsigned int *percent);

    int   slider_bar_set_value(HWND hwnd, int value);
    int   slider_bar_set_percent(HWND hwnd, unsigned int percent);
    #else  /* _LG_ALONE_VERSION_ */
    #define  slider_bar_create(parent, gui_common_widget, gui_slider_bar)    in_slider_bar_create(parent, gui_common_widget, gui_slider_bar)

    #define  slider_bar_get_value(hwnd, value)                               in_slider_bar_get_value(hwnd, value)
    #define  slider_bar_get_percent(hwnd, percent)                           in_slider_bar_get_percent(hwnd, percent)

    #define  slider_bar_set_value(hwnd, value)                               in_slider_bar_set_value(hwnd, value)
    #define  slider_bar_set_percent(hwnd, percent)                           in_slider_bar_set_percent(hwnd, percent)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_SLIDER_BAR_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_SLIDER_BAR_WIDGET_HEADER__ */
