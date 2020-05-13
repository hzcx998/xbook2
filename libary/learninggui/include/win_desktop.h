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

#ifndef  __LGUI_WIN_DESKTOP_HEADER__
#define  __LGUI_WIN_DESKTOP_HEADER__

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
#include  <win_image.h>
#include  <win_type_widget.h>
#include  <win_dc.h>
#endif

#include  <message.h>


/* GUI_BACKGROUND_IMAGE */
struct  _GUI_BACKGROUND_IMAGE
{
    /* Background image flag */
    BINT          bimage_flag;

    /* Background image type */
    BINT          bimage_type;

    /* Background image align */
    BINT          bimage_align;

    /* Background image frame id */
    unsigned int  frame_id;

    /* Background image data */
    const  void  *pimage;
};
typedef	struct	_GUI_BACKGROUND_IMAGE  GUI_BACKGROUND_IMAGE;


#ifdef  _LG_WINDOW_

#ifdef  __cplusplus
extern  "C"
{
#endif

    extern   HWND   lhdesk;
    #define  HWND_DESKTOP   lhdesk



    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_

    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
    int  in_win_set_background_image(HWND hwnd, /* GUI_BACKGROUND_IMAGE */void *gui_background_image);
    int  in_win_clear_background_image(HWND hwnd);
    #endif

    #ifndef  _LG_ALONE_VERSION_
    int  win_set_background_image(HWND hwnd, /* GUI_BACKGROUND_IMAGE */void *gui_background_image);
    int  win_clear_background_image(HWND hwnd);
    #else
    #define  win_set_background_image(hwnd, gui_background_image)      in_win_set_background_image(hwnd, gui_background_image)
    #define  win_clear_background_image(hwnd)                          in_win_clear_background_image(hwnd)
    #endif

    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_DESKTOP_HEADER__ */
