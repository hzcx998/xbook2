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

#ifndef  __LGUI_WIN_INTERFACE_HEADER__
#define  __LGUI_WIN_INTERFACE_HEADER__

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


#ifdef  _LG_WINDOW_

#define  GET_FROM_HWND_FROM_MSG(msg)         (((GUI_MESSAGE *)(msg))->from_hwnd)
#define  GET_TO_HWND_FROM_MSG(msg)           (((GUI_MESSAGE *)(msg))->to_hwnd)

#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal function */

    /* Window DC default font */
    extern  volatile  GUI_FONT  *lwdcft;

    /* Client DC default font */
    extern  volatile  GUI_FONT  *lcdcft;

    /* Window border width */
    extern  unsigned int  win_border_width;

    /* Window border color */
    extern  GUI_COLOR  win_border_active_color;
    extern  GUI_COLOR  win_border_inactive_color;

    /* Window client default color */
    extern  GUI_COLOR     win_client_color;
 

    #ifdef  _LG_WINDOW_BAR_
    /* Window bar height */
    extern  unsigned int  win_bar_height;

    /* Window active default color */
    extern  GUI_COLOR  win_bar_active_color;
 
    /* Window inactive default color */
    extern  GUI_COLOR  win_bar_inactive_color;

    /* Window title default color */
    extern  GUI_COLOR  win_title_color;

    /* Window system button width */
    extern  unsigned int  win_system_button_width;

    /* Window system button border color */
    extern  GUI_COLOR  win_system_button_border_color;

    /* Window close system button active color */
    extern  GUI_COLOR  close_button_active_color;
    /* Window close system button inactive color */
    extern  GUI_COLOR  close_button_inactive_color;
    /* Window max system button active color */
    extern  GUI_COLOR  max_button_active_color;
    /* Window max system button inactive color */
    extern  GUI_COLOR  max_button_inactive_color;
    /* Window min system button active color */
    extern  GUI_COLOR  min_button_active_color;
    /* Window max system button inactive color */
    extern  GUI_COLOR  min_button_inactive_color;
    #endif  /* _LG_WINDOW_BAR_ */

    #ifdef  _LG_SCROLL_BAR_
    /* Window scroll bar height or width */
    extern  unsigned int  lsbahw;
    #endif  /* _LG_SCROOL_BAR_ */


    int  in_init_window(void);

    int  in_set_default_window_by_key(int  key);

    /* Internal function end */


    int  in_move_window(/* HWND hwnd */ void *hwnd, int dx, int dy);

    int  in_win_show(void *hwnd);
    int  in_win_hide(void *hwnd);

    int  in_win_close(void *hwnd);
    int  in_win_close_all(void);

    int  in_win_is_visual(void *hwnd);

    int  in_win_enable(void *hwnd);
    int  in_win_disable(void *hwnd);

    int  in_win_is_enable(void *hwnd);
    int  in_win_is_disable(void *hwnd);

    int  in_win_is_ghost(void *hwnd);

    int  in_win_maxize(void *hwnd);

    int  in_win_move(void *hwnd, int dx, int dy);
    int  in_win_resize(void *hwnd, int left, int top, int right, int bottom);

    int  in_win_set_focus(void *hwnd);
    int  in_win_is_focus(void *hwnd);
    HWND in_win_get_focus(void);

    int  in_win_set_default(void *hwnd);
    int  in_win_is_default(void *hwnd);
    HWND in_win_get_default(void);

    int  in_win_enable_default_focus(void *hwnd);
    int  in_win_disable_default_focus(void *hwnd);

    int  in_win_bring_to_top(const void *hwnd);

    int  in_get_hwnd_by_id(const void *hwnd, unsigned int id);

    int  in_win_enable_erase_back(void *hwnd);
    int  in_win_disable_erase_back(void *hwnd);


    #ifndef  _LG_ALONE_VERSION_
    int  move_window(/* HWND hwnd */ void *hwnd, int dx, int dy);

    int  win_show(void *hwnd);
    int  win_hide(void *hwnd);

    int  win_close(void *hwnd);
    int  win_close_all(void);

    int  win_is_visual(void *hwnd);

    int  win_enable(void *hwnd);
    int  win_disable(void *hwnd);

    int  win_is_enable(void *hwnd);
    int  win_is_disable(void *hwnd);

    int  win_is_ghost(void *hwnd);

    int  win_maxize(void *hwnd);

    int  win_move(void *hwnd, int dx, int dy);
    int  win_resize(void *hwnd, int left, int top, int right, int bottom);

    int  win_set_focus(void *hwnd);
    int  win_is_focus(void *hwnd);
    HWND win_get_focus(void);

    int  win_set_default(void *hwnd);
    int  win_is_default(void *hwnd);
    HWND win_get_default(void);

    int  win_enable_default_focus(void *hwnd);
    int  win_disable_default_focus(void *hwnd);

    int  win_bring_to_top(const void *hwnd);

    HWND win_get_hwnd_by_id(const void *hwnd, unsigned int id);

    int  win_enable_erase_back(void *hwnd);
    int  win_disable_erase_back(void *hwnd);

    #else  /* _LG_ALONE_VERSION_ */

    #define  move_window(hwnd, dx, dy)                        in_move_window(hwnd, dx, dy)

    #define  win_show(hwnd)                                   in_win_show(hwnd)
    #define  win_hide(hwnd)                                   in_win_hide(hwnd)
    #define  win_close(hwnd)                                  in_win_close(hwnd)

    #define  win_close_all()                                  in_win_close_all()

    #define  win_enable(hwnd)                                 in_win_enable(hwnd)
    #define  win_disable(hwnd)                                in_win_disable(hwnd)

    #define  win_is_enable(hwnd)                              in_win_is_enable(hwnd)
    #define  win_is_disable(hwnd)                             in_win_is_disable(hwnd)

    #define  win_is_ghost(hwnd)                               in_win_is_ghost(hwnd)

    #define  win_maxize(hwnd)                                 in_win_maxize(hwnd)

    #define  win_move(hwnd, dx, dy)                           in_win_move(hwnd, dx, dy)
    #define  win_resize(hwnd, left, top, right, bottom)       in_win_resize(hwnd, left, top, right, bottom)

    #define  win_set_focus(hwnd)                              in_win_set_focus(hwnd)
    #define  win_is_focus(hwnd)                               in_win_is_focus(hwnd)
    #define  win_get_focus()                                  in_win_get_focus()

    #define  win_set_default(hwnd)                            in_win_set_default(hwnd)
    #define  win_is_default(hwnd)                             in_win_is_default(hwnd)
    #define  win_get_default()                                in_win_get_default()

    #define  win_bring_to_top(hwnd)                           in_win_bring_to_top(hwnd)

    #define  win_get_hwnd_by_id(hwnd, id)                     in_win_get_hwnd_by_id(hwnd, id)

    #define  win_enable_erase_back(hwnd)                      in_win_enable_erase_back(hwnd)
    #define  win_disable_erase_back(hwnd)                     in_win_disable_erase_back(hwnd)

    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

    #define  win_set_border_width(width)                       (win_border_width = width)
    #define  win_set_border_active_color(width)                (win_border_active_color = color)
    #define  win_set_border_inactive_color(width)              (win_border_inactive_color = color)

    #define  win_set_client_color(color)                       (win_client_color = color)

#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_INTERFACE_HEADER__ */
