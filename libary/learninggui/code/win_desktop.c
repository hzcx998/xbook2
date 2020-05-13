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

#include  <string.h>

#include  <lock.h>

#include  <default.h>

#include  <win_tools.h>
#include  <win_interface.h>

#include  <win_desktop.h>
#include  <win_widget.h>

#include  "win_desktop_in.h"




#ifdef  _LG_WINDOW_

/* Temp GUI_DC buffer */
extern  volatile  GUI_DC              ltdc;

/* Temp common widget buffer */
extern  volatile  GUI_COMMON_WIDGET   ltcomm;



/* Desktop HWND */
HWND      lhdesk  = NULL;

/* Desktop GUI_WND */
static  GUI_WND  ldtwnd  = { {0} };



int  in_desktop_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    if (msg == NULL)
        return  0;

    switch ( MESSAGE_GET_ID(msg) )
    {
        case  MSG_PAINT:
            in_paint_widget_back(lhdesk);
            break;

        default:
            break;
    }

    return  1;
}

int  in_desktop_init(void)
{

    lhdesk  = (HWND)(&ldtwnd);

    /* ?? */
    memset((void *)(&ltdc), 0, sizeof(GUI_DC));
    in_hdc_basic_set_default((HDC)(&ltdc));

    memset((void *)(&ltcomm), 0, sizeof(GUI_COMMON_WIDGET));
    ltcomm.type              = 0;
    ltcomm.id                = 0;

    ltcomm.style             = VISUAL_STYLE | ENABLE_STYLE;
    ltcomm.ext_style         = 0;

    /* Change default background color */
    in_hdc_set_back_color((HDC)&(ltdc), GUI_BLUE);

    ltcomm.win_dc            = ltdc;
    ltcomm.client_dc         = ltdc;

    ltcomm.is_in_callback    = 1;
    ltcomm.in_callback       = in_desktop_callback;

    ltcomm.is_app_callback    = 0;
    ltcomm.app_callback       = NULL;

    ldtwnd.common             = ltcomm;

    return  1;
}

    
#ifdef  _LG_WINDOW_BACKGROUND_IMAGE_

int in_win_set_background_image(HWND hwnd, /* GUI_BACKGROUND_IMAGE */ void *gui_background_image)
{
    HWND  p = hwnd;
    GUI_BACKGROUND_IMAGE  *pback = (GUI_BACKGROUND_IMAGE *)gui_background_image;


    if ( p == NULL )
        return  -1;
    if ( pback == NULL )
        return  -1;


    p->common.bimage_flag  = pback->bimage_flag;
    p->common.bimage_type  = pback->bimage_type;
    p->common.bimage_align = pback->bimage_align;
    p->common.pimage       = pback->pimage;

    in_win_message_send_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int win_set_background_image(HWND hwnd, /* GUI_BACKGROUND_IMAGE */void *gui_background_image)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_set_background_image(hwnd, gui_background_image);
    gui_unlock( );

    return  ret;
}
#endif

int in_win_clear_background_image(HWND hwnd)
{
    HWND  p = hwnd;


    if ( p == NULL )
        return  -1;

    p->common.bimage_flag     = 0;
    p->common.bimage_type     = 0;
    p->common.bimage_align    = 0;
    p->common.frame_id        = 0;
    p->common.pimage          = NULL;

    in_win_message_send_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int win_clear_background_image(HWND hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_clear_background_image(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

#endif /* _LG_WINDOW_BACKGROUND_IMAGE_ */

#endif  /* _LG_WINDOW_ */
