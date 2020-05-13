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

#ifndef  __LGUI_WINDOW_BAR_HEADER__
#define  __LGUI_WINDOW_BAR_HEADER__

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

#include  <text_ops.h>



#ifndef  MAX_WIN_TEXT_LEN
#define  MAX_WIN_TEXT_LEN                 32
#endif
#if  (MAX_WIN_TEXT_LEN < 1)
#undef   MAX_WIN_TEXT_LEN
#define  MAX_WIN_TEXT_LEN                 32
#endif


/* GUI_WINBAR */
struct  _GUI_WINBAR
{
    /* Status */
    unsigned int  status;

    /* Raw rect */
    GUI_RECT      raw_rect;

    /* Window bar rect */
    GUI_RECT      bar_rect;

    /* Window title rect */
    GUI_RECT      title_rect;

    /* Close button rect */
    GUI_RECT      close_rect;

    /* Maxize button rect */
    GUI_RECT      max_rect;

    /* Minimize button rect */
    GUI_RECT      min_rect;
	
    /* Window text */	
    TCHAR         text[MAX_WIN_TEXT_LEN+1];

    /* Window text len */	
    unsigned int  len;
};
typedef	struct	_GUI_WINBAR   GUI_WINBAR;


#ifdef  __cplusplus
extern  "C"
{
#endif

    #ifdef  _LG_WINDOW_BAR_

    /* Internal function */
    /* Temp window bar buffer */
    extern  volatile  GUI_WINBAR   ltwbar;

    GUI_WINBAR  *in_win_get_window_bar(/* HWND hwnd */ void *hwnd);
    int  in_win_paint_window_bar(/* HWND hwnd */ void *hwnd);

    int  in_win_normal(void *hwnd);
    int  in_win_set_window_bar_text(void *hwnd, TCHAR *text, unsigned int text_len);
    int  in_win_get_window_bar_text(void *hwnd, TCHAR *text, unsigned int *text_len);
    /* Internal function end */


    #ifdef  _LG_WINDOW_BAR_
    int  in_win_normal(void *hwnd);
    int  in_win_set_window_bar_text(void *hwnd, TCHAR *text, unsigned int  text_len);
    int  in_win_get_window_bar_text(void *hwnd, TCHAR *text, unsigned int  *text_len);
    #endif

    #ifndef  _LG_ALONE_VERSION_
    int  win_normal(void *hwnd);
    int  win_set_window_bar_text(void *hwnd, TCHAR *text, unsigned int text_len);
    int  win_get_window_bar_text(void *hwnd, TCHAR *text, unsigned int *text_len);
    #else
    #define  win_normal(hwnd)                              in_win_normal(hwnd)
    #define  win_set_window_bar_text(hwnd,text,text_len)   in_win_set_window_bar_text(hwnd,text,text_len)
    #define  win_get_window_bar_text(hwnd,text,text_len)   in_win_get_window_bar_text(hwnd,text,text_len)
    #endif 

    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* __LGUI_WINDOW_BAR_HEADER__ */
