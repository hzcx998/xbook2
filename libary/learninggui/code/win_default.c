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

#include  <win_interface.h>
#include  <win_desktop.h>

#include  "win_default_in.h"



#ifdef  _LG_WINDOW_

int  in_win_set_window_default_font(/* const GUI_FONT *font */ const void *font)
{
    if (font == NULL)
        return  -1;

    lwdcft = (GUI_FONT *)font;
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_set_window_default_font(/* const GUI_FONT *font */ const void *font)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_set_window_default_font(font);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_win_set_client_default_font(/* const GUI_FONT *font */ const void *font)
{
    if (font == NULL)
        return  -1;

    lcdcft = (GUI_FONT *)font;
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_set_client_default_font(/* const GUI_FONT *font */ const void *font)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_set_client_default_font(font);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_win_set_current_color_group(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;


    if ( p == NULL )
        return  -1;
    if ( (p->head.parent) == NULL )
        return  -1;


    if ( ((p->common.style)&ENABLE_STYLE) != ENABLE_STYLE )
    {
        p->common.win_dc.cur_group    = DISABLED_GROUP;
        p->common.client_dc.cur_group = DISABLED_GROUP;
        return  1;
    }

    if (in_win_is_focus(p) > 0)
    {
        p->common.win_dc.cur_group    = ACTIVE_GROUP;
        p->common.client_dc.cur_group = ACTIVE_GROUP;
        return  1;
    }

    p->common.win_dc.cur_group    = INACTIVE_GROUP;
    p->common.client_dc.cur_group = INACTIVE_GROUP;

    return  1;
}

#endif  /* _LG_WINDOW_ */
