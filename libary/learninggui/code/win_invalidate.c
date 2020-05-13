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

#include  <rect_ops.h>

#include  <win_desktop.h>
#include  <win_interface.h>



#ifdef  _LG_WINDOW_

/* Invalindate Window numbers */
volatile  unsigned int  linvan = 0;



/* Absolution coord */
/* Only this window */
int  in_win_invalidate_rect_abs(/* HWND hwnd */ void *hwnd, const /* GUI_RECT */ void *rect)
{
    HWND      p         = (HWND)hwnd;
    HWND      parent_p  = (HWND)hwnd;
    GUI_RECT  temp_rect;
    int       ret = 0;



    if ( p == NULL )
        return  -1;


    /* 20170622 */
    ret = in_win_is_ghost(p);
    if ( ret > 0 )
        return  -1;

    /* Set temp_rect */
    if ( rect == NULL )
    {
        temp_rect = p->common.win_dc.rect;
    } else {
        temp_rect = *((GUI_RECT *)rect);
        if ( in_is_intersect_rect(&(p->common.win_dc.rect), &temp_rect) < 1 )
           return  -1;

        in_intersect_rect(&temp_rect, &(p->common.win_dc.rect), &temp_rect);
    }

    parent_p = p->head.parent;
    while ( (parent_p != NULL)&&(parent_p != HWND_DESKTOP) )
    {
        if ( in_is_intersect_rect(&(parent_p->common.client_dc.rect), &temp_rect) < 1 )
           return  -1;

        in_intersect_rect(&temp_rect, &(parent_p->common.client_dc.rect), &temp_rect);
        parent_p = parent_p->head.parent;
    }
   
    /* Set invalidate rect */
    if ( (p->common.invalidate_flag) < 1 )
    {
        p->common.invalidate_rect = temp_rect;
        p->common.invalidate_flag = 1;
        linvan++;
    } else {
        in_merge_rect(&(p->common.invalidate_rect), &(p->common.invalidate_rect), &temp_rect);
    }

    return  1;
}

/* Releatived coord */
int  in_win_invalidate_rect(/* HWND hwnd */ void *hwnd, const /* GUI_RECT */ void *rect)
{
    HWND      p = (HWND)hwnd;
    GUI_RECT  rect_abs;
    int       ret;


    if ( p == NULL )
        return  -1;


    if ( rect == NULL )
    {
        rect_abs = p->common.win_dc.rect;
    } else {
        /* ??  win_dc */
        rect_abs         = *((GUI_RECT *)rect);
        rect_abs.left   += p->common.client_dc.rect.left;
        rect_abs.right  += p->common.client_dc.rect.left;
        rect_abs.top    += p->common.client_dc.rect.top;
        rect_abs.bottom += p->common.client_dc.rect.top;
    }

    ret = in_win_invalidate_rect_abs(p, &rect_abs);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
/* Releatived coord */
int  win_invalidate_rect(/* HWND hwnd */ void *hwnd, const /* GUI_RECT */ void *rect)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_invalidate_rect(hwnd, rect);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_invalidate_area_abs(const /* GUI_RECT */ void *rect)
{
    HWND  p = NULL;


    if ( rect == NULL )
        return  -1;

    p = HWND_DESKTOP;
    while ( p != NULL )
    {
        in_win_invalidate_rect_abs(p, rect);
        p = p->head.next;
    }

    return  1;
}

/* ?? Main or child ?? */
int  in_win_invalidate_below_area_abs( /* HWND hwnd */ void *hwnd, const /* GUI_RECT */ void *rect)
{
    HWND  p      = (HWND)hwnd;
    HWND  temp_p = NULL;


    if (p == NULL)
        return  -1;
    if ( rect == NULL )
        return  -1;


    temp_p = p;
    while ( (temp_p != NULL)&&((temp_p->head.parent) != HWND_DESKTOP) )
        temp_p = temp_p->head.parent;
 
    while ( temp_p != NULL )
    {
        in_win_invalidate_rect_abs(temp_p, rect);
        temp_p = temp_p->head.prev;
    }

    return  1;
}

int  in_win_paint_invalidate_recursion(/* HWND hwnd */ void *hwnd)
{
    HWND  p      = (HWND)hwnd;
    GUI_MESSAGE  msg;
    int          ret = 0;


    if ( linvan  < 1 )
        return  -1;

    memset(&msg, 0, sizeof(GUI_MESSAGE));
    msg.id            = MSG_PAINT;
    msg.callback_flag = HWND_APP_CALLBACK | HWND_IN_CALLBACK;

    while (p != NULL)
    {
        if ( (p->common.invalidate_flag) > 0 )
        {
            msg.to_hwnd = p;

            ret = 1;
            if ( (p->common.is_app_callback) > 0 )
            {
                gui_unlock();
                ret = p->common.app_callback(&msg);
                gui_lock();
            }

            if ( (ret > 0) && ((p->common.is_in_callback) > 0) )
                p->common.in_callback(&msg);

            p->common.invalidate_flag = 0;
            linvan--;
            if ( linvan < 1 )
            {
                linvan = 0;
                return 1;
            }
        }

        if ( (p->head.lc) != NULL )
            in_win_paint_invalidate_recursion(p->head.lc);

        p = p->head.prev;
    } 
            
    return  -1;
}
#endif  /* _LG_WINDOW_ */
