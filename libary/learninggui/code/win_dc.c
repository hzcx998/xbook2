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

#include  <win_desktop.h>


#ifdef  _LG_WINDOW_

HDC  in_hdc_get_window(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;


    if (p == NULL)
        return  NULL;

    if ( (p->common.win_dc.used) == GUI_USED )
        return  NULL;

    p->common.win_dc.used = GUI_USED;

    #ifdef  _LG_ALPHA_BLEND_
    p->common.win_dc.is_alpha_blend      = 0;
    p->common.win_dc.alpha_blend_op_mode = ALPHA_BLEND_OP_ADD;
    #endif

    return  &(p->common.win_dc);
}

#ifndef  _LG_ALONE_VERSION_
HDC  hdc_get_window(/* HWND  hwnd */ void *hwnd)
{
    HDC  hdc = NULL ;

    gui_lock( );
    hdc = in_hdc_get_window(hwnd);
    gui_unlock( );

    return  hdc;
}
#endif  /* _LG_ALONE_VERSION_ */

HDC  in_hdc_get_client(/* HWND  hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;


    if (p == NULL)
        return  NULL;

    if ( (p->common.client_dc.used) == GUI_USED )
        return  NULL;

    p->common.client_dc.used = GUI_USED;

    #ifdef  _LG_ALPHA_BLEND_
    p->common.client_dc.is_alpha_blend      = 0;
    p->common.client_dc.alpha_blend_op_mode = ALPHA_BLEND_OP_ADD;
    #endif

    return  &(p->common.client_dc);
}

#ifndef  _LG_ALONE_VERSION_
HDC  hdc_get_client(/* HWND  hwnd */ void *hwnd)
{
    HDC  hdc = NULL;

    gui_lock( );
    hdc = in_hdc_get_client(hwnd);
    gui_unlock( );

    return  hdc;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_hdc_release_win(/* HWND hwnd */ void *hwnd, HDC hdc)
{
    HWND  p = (HWND)hwnd;


    if (p == NULL)
        return  -1;

    if ( (&(p->common.win_dc)) == hdc )
    {
        p->common.win_dc.used = GUI_UNUSED;

        #ifdef  _LG_ALPHA_BLEND_
        p->common.win_dc.is_alpha_blend      = 0;
        p->common.win_dc.alpha_blend_op_mode = ALPHA_BLEND_OP_ADD;
        #endif

        return  1;
    }

    if ( (&(p->common.client_dc)) == hdc )
    {
        p->common.client_dc.used = GUI_UNUSED;

        #ifdef  _LG_ALPHA_BLEND_
        p->common.client_dc.is_alpha_blend      = 0;
        p->common.client_dc.alpha_blend_op_mode = ALPHA_BLEND_OP_ADD;
        #endif

        return  1;
    }

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_release_win(/* HWND hwnd */ void *hwnd, HDC hdc)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_release_win(hwnd, hdc);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_WINDOW_ */
