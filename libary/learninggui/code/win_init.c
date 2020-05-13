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

#include  <win_tools.h>
#include  <win_desktop.h>
#include  <win_interface.h>

#include  "win_desktop_in.h"
#include  "win_arithmetic_in.h"
#include  "win_widget_group_in.h"


#ifdef  _LG_WINDOW_

static  int  in_init_window_manager(void)
{
    lhlist = HWND_DESKTOP;
    lhfocu = HWND_DESKTOP;
    #ifdef   _LG_MTJT_
    lhmtjt = HWND_DESKTOP;
    #endif
    lhdefa = HWND_DESKTOP;

    in_win_arithmetic_init();

    return  1;
}

int  in_init_window(void)
{
    in_desktop_init();
    in_init_window_manager();

    #ifdef  _LG_WIDGET_GROUP_
    in_widget_group_init();
    #endif

    in_win_message_send_ext(HWND_DESKTOP, MSG_PAINT, HWND_IN_CALLBACK);

    return  1;
}

#endif  /* _LG_WINDOW_ */
