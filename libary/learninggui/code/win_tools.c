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

#include  <stdlib.h>
#include  <string.h>

#include  <lock.h>

#include  <win_tools.h>
#include  <win_callback.h>
#include  <win_interface.h>

#include  <win_desktop.h>



#ifdef  _LG_WINDOW_
int  in_win_message_post_ext(/* HWND hwnd */ void *hwnd, UINT message_id, int flag)
{
    GUI_MESSAGE   msg;


    if ( hwnd == NULL )
        return  -1;

    memset((void *)(&msg), 0, sizeof(GUI_MESSAGE));
    msg.id             = message_id;
    msg.to_hwnd        = hwnd;
    msg.callback_flag  = flag & HWND_CALLBACK_MASK;
    in_message_post(&msg);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_message_post_ext(/* HWND hwnd */ void *hwnd, UINT message_id, int flag)
{
    int  ret = 0;

    gui_lock( );
    ret = win_message_post_ext(hwnd, message_id, flag);
    gui_unlock( );

    return  ret;
}
#endif


int  in_win_message_send_ext(/* HWND hwnd */ void *hwnd, UINT message_id, int flag)
{
    GUI_MESSAGE   msg;


    if ( hwnd == NULL )
        return  -1;

    memset((void *)(&msg), 0, sizeof(GUI_MESSAGE));
    msg.id             = message_id;
    msg.to_hwnd        = hwnd;
    msg.callback_flag  = flag & HWND_CALLBACK_MASK;
    in_message_send(&msg);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_message_send_ext(/* HWND hwnd */ void *hwnd, UINT message_id, int flag)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_message_send_ext(hwnd, message_id, flag);
    gui_unlock( );

    return  ret;
}
#endif


int  in_make_callback_message(/* HWND hwnd */ void *hwnd, UINT message_id, int flag)
{
    GUI_MESSAGE   msg;


    if ( hwnd == NULL )
        return  -1;

    memset((void *)(&msg), 0, sizeof(GUI_MESSAGE));
    msg.id             = message_id;
    msg.to_hwnd        = hwnd;
    msg.callback_flag  = flag & HWND_CALLBACK_MASK;

    in_callback_to_hwnd_message(&msg);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  make_callback_message(/* HWND hwnd */ void *hwnd, UINT message_id, int flag)
{
    int  ret = 0;

    gui_lock( );
    ret = in_make_callback_message(hwnd, message_id, flag);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_WINDOW_ */
