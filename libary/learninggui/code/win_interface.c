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

#include  <lgconst.h>

#include  <lock.h>
#include  <cursor.h>

#include  <keyboard.h>

#include  <win_tools.h>
#include  <win_callback.h>
#include  <win_invalidate.h>
#include  <win_desktop.h>
#include  <win_widget.h>
#include  <win_push_button.h>
#include  <win_interface.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_

/* Window DC default font */
volatile  GUI_FONT  *lwdcft             = NULL;

/* Client DC default font */
volatile  GUI_FONT  *lcdcft             = NULL;

/* Window border width */
unsigned int  win_border_width          = WIN_BORDER_WIDTH;

#ifdef  _LG_WINDOW_BAR_
/* Window bar height */
unsigned int  win_bar_height            = WIN_BAR_HEIGHT;

/* Window system button width */
unsigned int  win_system_button_width   = SYSTEM_BTN_WIDTH;

#endif  /* _LG_WINDOW_BAR_ */

#ifdef  _LG_SCROLL_BAR_
/* Window scroll bar height or width */
unsigned int  lsbahw                    = SCBAR_HEIGHT_WIDTH;
#endif  /* _LG_SCROLL_BAR_ */


int  in_win_show(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;

    if ( p == NULL )
        return  -1;

    p->common.style |= VISUAL_STYLE;

    in_win_set_focus(p);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_show(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_show(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_hide(/* HWND hwnd */ void *hwnd)
{
    HWND   p = (HWND)hwnd;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;


    if ( ((p->common.style)&&VISUAL_STYLE) != VISUAL_STYLE)
        return  0;

    p->common.style &= ~VISUAL_STYLE;
    in_win_invalidate_below_area_abs(p, &(p->common.win_dc.rect));

    in_win_delete_hide_comm(p);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_hide(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_hide(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_close(/* HWND hwnd */ void *hwnd)
{
    return  in_win_delete(hwnd);
}

#ifndef  _LG_ALONE_VERSION_
int  win_close(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_close(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_close_all(void)
{
    HWND  p      = NULL;
    HWND  temp_p = NULL;


    p      = lhlist;
    temp_p = p;
    while ( (temp_p != NULL)&&(temp_p != HWND_DESKTOP) )
    {
        p = p->head.prev;
        in_win_delete(temp_p);
        temp_p = p;
    }

    HWND_DESKTOP->head.prev   = NULL;
    HWND_DESKTOP->head.next   = NULL;
    HWND_DESKTOP->head.parent = NULL;

    lhlist = HWND_DESKTOP;
    lhfocu = HWND_DESKTOP;
    lhdefa = HWND_DESKTOP;
    #ifdef   _LG_MTJT_
    lhmtjt = HWND_DESKTOP;
    #endif

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_close_all(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_close_all();
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_is_visual(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;
    HWND  parent_p = NULL;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  1;

    if ( ((p->common.style)&VISUAL_STYLE) == 0 )
        return  -1;


    parent_p = p->head.parent;
    while ( (parent_p != NULL)&&(parent_p != HWND_DESKTOP) )
    {
        if ( ((parent_p->common.style)&VISUAL_STYLE) == 0 )
            return  -1;

        parent_p = parent_p->head.parent;        
    }

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_is_visual(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_is_visual(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_enable(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;

        
    if ( ((p->common.style)&ENABLE_STYLE) == ENABLE_STYLE )
        return  1;

    p->common.style |= ENABLE_STYLE;
    in_win_invalidate(p);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_enable(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_enable(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_disable(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;


    /* 20170607 ?? */
    if (p == lhfocu)
        lhfocu = HWND_DESKTOP;

    #ifdef   _LG_MTJT_
    if (p == lhmtjt)
        lhmtjt = HWND_DESKTOP;
    #endif

    if ( p== lhdefa )
        lhdefa = HWND_DESKTOP;


    p->common.style &= ~ENABLE_STYLE;
    in_win_invalidate(p);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_disable(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_disable(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_win_is_enable(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;
    HWND  parent_p = NULL;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  1;


    if ( ((p->common.style)&ENABLE_STYLE) == 0 )
        return  -1;


    parent_p = p->head.parent;
    while ( (parent_p != NULL)&&(parent_p != HWND_DESKTOP) )
    {
        if ( ((parent_p->common.style)&ENABLE_STYLE) == 0 )
            return  -1;

        parent_p = parent_p->head.parent;        
    }

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_is_enable(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_is_enable(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_win_is_disable(/* HWND hwnd */ void *hwnd)
{
    HWND  p  = (HWND)hwnd;
    HWND  parent_p = NULL;


    if ( p == NULL )
        return  1;
    if ( p == HWND_DESKTOP)
        return  -1;


    if ( ((p->common.style)&ENABLE_STYLE) == 0 )
        return  1;

    parent_p = p->head.parent;
    while ( (parent_p != NULL)&&(parent_p != HWND_DESKTOP) )
    {
        if ( ((parent_p->common.style)&ENABLE_STYLE) == 0 )
            return  1;

        parent_p = parent_p->head.parent;        
    }

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_is_disable(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_is_disable(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_win_is_ghost(/* HWND hwnd */ void *hwnd)
{
    #ifdef  _LG_PUSH_BUTTON_WIDGET_
    return  in_push_button_is_ghost(hwnd);
    #else
    return  -1;
    #endif
}

#ifndef  _LG_ALONE_VERSION_
int  win_is_ghost(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_is_ghost(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

int  in_win_maxize(/* HWND hwnd */ void *hwnd)
{
    HWND         p      = (HWND)hwnd;
    HWND         temp_p = NULL;
    HDC          hdc;
    GUI_RECT     rect;
    GUI_WINBAR  *wbar = NULL;
    int          dx;
    int          dy;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;
    if ( in_win_has(p) < 1 )
        return  -1;

    /* Update window rect */
    hdc = in_hdc_get_window(p);
    rect = hdc->rect;
    dx = rect.left - (p->head.parent)->common.win_dc.rect.left;
    dy = rect.top - (p->head.parent)->common.win_dc.rect.top;
    hdc->rect = (p->head.parent)->common.win_dc.rect;
    rect = hdc->rect;
    in_hdc_release_win(p, hdc);

    /* Update window bar rect */
    #ifdef  _LG_WINDOW_BAR_
    if ( IS_WINBAR_WIDGET(&(p->common)) )
    {
        wbar = in_win_get_window_bar(p);
        if ( wbar == NULL )
            goto  UPDATE_CLIENT_RECT;

        wbar->status = WINDOW_MAX_STATUS;

        if ( win_bar_height < IN_MIN_WINDOW_BAR_HEIGHT )
            win_bar_height = IN_MIN_WINDOW_BAR_HEIGHT;
        if ( win_system_button_width < IN_MIN_SYSTEM_BUTTON_WIDTH )
            win_system_button_width = IN_MIN_SYSTEM_BUTTON_WIDTH;

        if (IS_BORDER_WIDGET(&(p->common)))
        {
            rect.left               += win_border_width;
            rect.right              -= win_border_width;
            rect.top                += win_border_width;
        }

        rect.bottom                  = rect.top + (win_bar_height - 1);
        wbar->bar_rect                = rect;
        wbar->title_rect              = wbar->bar_rect;
        wbar->title_rect.right       -= IN_SYSTEM_BUTTON_H_DISTANCE;

        wbar->close_rect.left         = -1;
        wbar->close_rect.right        = -1;
        wbar->close_rect.top          = -1;
        wbar->close_rect.bottom       = -1;

        wbar->max_rect.left           = -1;
        wbar->max_rect.right          = -1;
        wbar->max_rect.top            = -1;
        wbar->max_rect.bottom         = -1;

        wbar->min_rect.left           = -1;
        wbar->min_rect.right          = -1;
        wbar->min_rect.top            = -1;
        wbar->min_rect.bottom         = -1;

        rect.right                  -= IN_SYSTEM_BUTTON_H_DISTANCE;
        rect.top                    += IN_SYSTEM_BUTTON_VT_DISTANCE;
        rect.bottom                 -= IN_SYSTEM_BUTTON_VB_DISTANCE;

        if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
        {
            wbar->close_rect               = rect;
            wbar->close_rect.left          = wbar->close_rect.right - (win_system_button_width-1);
        } 

        if ( IS_MAX_BTN_WINBAR(&(p->common)) )
        {
            wbar->max_rect                 = rect;
            if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
                wbar->max_rect.right      -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

            wbar->max_rect.left            = wbar->max_rect.right - (win_system_button_width-1);
        }

        if ( IS_MIN_BTN_WINBAR(&(p->common)) )
        {
            wbar->min_rect                 = rect;
            if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
                wbar->min_rect.right      -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

            if ( IS_MAX_BTN_WINBAR(&(p->common)) )
                wbar->min_rect.right      -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

            wbar->min_rect.left            = wbar->min_rect.right - (win_system_button_width-1);
        }
    }
    UPDATE_CLIENT_RECT:
    #endif  /* _LG_WINDOW_BAR_ */

    /* Update client rect */
    rect                                = p->common.win_dc.rect;
    if  (IS_BORDER_WIDGET(&(p->common)))
    {
        rect.left                      += win_border_width;
        rect.right                     -= win_border_width;
        rect.top                       += win_border_width;
        rect.bottom                    -= win_border_width;
    }
    if (IS_WINBAR_WIDGET(&(p->common)))
        rect.top                       += win_bar_height;
    p->common.client_dc.rect            = rect;

    /* Update all chlidren rect */
    temp_p = p->head.fc;
    while ( temp_p != NULL )
    {
        in_win_move(temp_p, -dx, -dy);
        temp_p = temp_p->head.next;
    }     

    /* Invalidate window */
    if ( in_win_is_visual(p) )
        in_win_invalidate(p);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_maxize(/* HWND hwnd */ void *hwnd)
{
    HWND  p   = (HWND)hwnd;
    int   ret = 0;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;
    if ( in_win_has(p) < 1 )
        return  -1;

    gui_lock( );
    ret = in_win_maxize(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

static  int  in_win_move_recursion(/* HWND hwnd */ void *hwnd, int dx, int dy, int flag)
{
    HWND         p     = (HWND)hwnd;
    HDC          hdc;
    GUI_RECT     rect;
    GUI_WINBAR  *wbar  = NULL;
 

    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;


    while( p != NULL )
    {
        /* Update window rect */
        hdc = in_hdc_get_window(p);
        rect = hdc->rect;
      
        rect.left   += dx;
        rect.right  += dx;
        rect.top    += dy;
        rect.bottom += dy;

        hdc->rect = rect;
        in_hdc_release_win(p, hdc);

        /* Update window bar rect */
        #ifdef  _LG_WINDOW_BAR_
        if ( IS_WINBAR_WIDGET(&(p->common)) )
        {
            wbar  = in_win_get_window_bar(p);
            if ( wbar == NULL )
                goto  UPDATE_CLIENT_RECT;

            /* raw_rect */
            rect                    = wbar->raw_rect;
            rect.left              += dx;
            rect.right             += dx;
            rect.top               += dy;
            rect.bottom            += dy;
            wbar->raw_rect           = rect;

            /* bar_rect */
            rect                    = wbar->bar_rect;
            rect.left              += dx;
            rect.right             += dx;
            rect.top               += dy;
            rect.bottom            += dy;
            wbar->bar_rect           = rect;

            /* titile_rect */
            rect                    = wbar->title_rect;
            rect.left              += dx;
            rect.right             += dx;
            rect.top               += dy;
            rect.bottom            += dy;
            wbar->title_rect         = rect;

            /* System close button rect */
            if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
            {
                rect                = wbar->close_rect;
                rect.left          += dx;
                rect.right         += dx;
                rect.top           += dy;
                rect.bottom        += dy;
                wbar->close_rect     = rect;
            } 

            /* System max button rect */
            if ( IS_MAX_BTN_WINBAR(&(p->common)) )
            {
                rect                = wbar->max_rect;
                rect.left          += dx;
                rect.right         += dx;
                rect.top           += dy;
                rect.bottom        += dy;
                wbar->max_rect       = rect;
            }

            /* System mix button rect */
            if ( IS_MIN_BTN_WINBAR(&(p->common)) )
            {
                rect                = wbar->min_rect;
                rect.left          += dx;
                rect.right         += dx;
                rect.top           += dy;
                rect.bottom        += dy;
                wbar->min_rect       = rect;
            }
        }

        UPDATE_CLIENT_RECT:
        #endif  /* _LG_WINDOW_BAR_ */

        /* Update client rect */
        hdc = in_hdc_get_client(p);
        rect = hdc->rect;

        rect.left   += dx;
        rect.right  += dx;
        rect.top    += dy;
        rect.bottom += dy;

        hdc->rect = rect;
        in_hdc_release_win(p, hdc);


        if ( (p->head.fc) != NULL )
            in_win_move_recursion(p->head.fc, dx, dy, 0);

        if ( flag )
            break;

        p = p->head.next;
    }

    return  1;
}

int  in_win_move(/* HWND hwnd */ void *hwnd, int dx, int dy)
{
    HWND  p = (HWND)hwnd;
 

    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;
    if ( in_win_has(p) < 1 )
        return  -1;


    /* Invalidate old window area */
    if ( in_win_is_visual(p) )
        in_win_invalidate_below_area_abs(p, &(p->common.win_dc.rect));

    in_win_move_recursion(p, dx, dy, 1);

    /* Invalidate new window area */
    if ( in_win_is_visual(p) )
        in_win_invalidate_below_area_abs(p, &(p->common.win_dc.rect));

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_move(/* HWND hwnd */ void *hwnd, int dx, int dy)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_move(hwnd, dx, dy);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_resize(/* HWND hwnd */ void *hwnd, int left, int top, int right, int bottom)
{
    HWND         p      = (HWND)hwnd;
    HWND         temp_p = NULL;
    HDC          hdc;
    GUI_RECT     rect;
    GUI_WINBAR  *wbar   = NULL;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;
    if ( in_win_has(p) < 1 )
        return  -1;


    if ( left > right )
        return  -1;
    if ( top > bottom )
        return  -1;

    if ( (right - left) < IN_MIN_WIDGET_WIDTH )
        right = left + IN_MIN_WIDGET_WIDTH;


    /* Invalidate window */
    if ( in_win_is_visual(p) )
        in_win_invalidate_below_area_abs(p, &(p->common.win_dc.rect));


    /* Update window rect */
    hdc = in_hdc_get_window(p);
    rect = hdc->rect;

    rect.left   += left;
    rect.top    += top;
    rect.right   = rect.left + right - left;
    rect.bottom  = rect.top  + bottom - top;

    hdc->rect = rect;
    in_hdc_release_win(p, hdc);


    /* Update window bar rect */
    #ifdef  _LG_WINDOW_BAR_
    wbar = in_win_get_window_bar(p);
    if ( wbar == NULL )
        goto  UPDATE_CLIENT_RECT;

    /* Bar rect and title rect */
    if ( win_bar_height < IN_MIN_WINDOW_BAR_HEIGHT )
        win_bar_height = IN_MIN_WINDOW_BAR_HEIGHT;

    if ( win_system_button_width < IN_MIN_SYSTEM_BUTTON_WIDTH )
        win_system_button_width = IN_MIN_SYSTEM_BUTTON_WIDTH;

    wbar->status                         = WINDOW_NORMAL_STATUS;
    wbar->raw_rect                       = rect;
    if (IS_BORDER_WIDGET(&(p->common)))
    {
        rect.left                      += win_border_width;
        rect.right                     -= win_border_width;
        rect.top                       += win_border_width;
    }
    rect.bottom                         = rect.top + win_bar_height;
    wbar->bar_rect                       = rect;
    wbar->title_rect                     = wbar->bar_rect;
    wbar->title_rect.right              -= IN_SYSTEM_BUTTON_H_DISTANCE;

    wbar->close_rect.left                = -1;
    wbar->close_rect.right               = -1;
    wbar->close_rect.top                 = -1;
    wbar->close_rect.bottom              = -1;

    wbar->max_rect.left                  = -1;
    wbar->max_rect.right                 = -1;
    wbar->max_rect.top                   = -1;
    wbar->max_rect.bottom                = -1;

    wbar->min_rect.left                  = -1;
    wbar->min_rect.right                 = -1;
    wbar->min_rect.top                   = -1;
    wbar->min_rect.bottom                = -1;

    rect.right                         -= IN_SYSTEM_BUTTON_H_DISTANCE;
    rect.top                           += IN_SYSTEM_BUTTON_VT_DISTANCE;
    rect.bottom                        -= IN_SYSTEM_BUTTON_VB_DISTANCE;

    /* System close button rect */
    if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
    {
        wbar->close_rect                  = rect;
        wbar->close_rect.left             = wbar->close_rect.right - (win_system_button_width-1);
    } 

    /* System max button rect */
    if ( IS_MAX_BTN_WINBAR(&(p->common)) )
    {
        wbar->max_rect                    = rect;
        if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
            wbar->max_rect.right         -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

        wbar->max_rect.left               = wbar->max_rect.right - (win_system_button_width-1);
    }

    /* System mix button rect */
    if ( IS_MIN_BTN_WINBAR(&(p->common)) )
    {
        wbar->min_rect                    = rect;
        if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
            wbar->min_rect.right         -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

        if ( IS_MAX_BTN_WINBAR(&(p->common)) )
            wbar->min_rect.right         -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

        wbar->min_rect.left               = wbar->min_rect.right - (win_system_button_width-1);
    }

    UPDATE_CLIENT_RECT:
    #endif  /* _LG_WINDOW_BAR_ */

    /* Update client rect */
    rect                                = p->common.win_dc.rect;
    if  (IS_BORDER_WIDGET(&(p->common)))
    {
        rect.left                      += win_border_width;
        rect.right                     -= win_border_width;
        rect.top                       += win_border_width;
        rect.bottom                    -= win_border_width;
    }

    if (IS_WINBAR_WIDGET(&(p->common)))
        rect.top                       += win_bar_height;

    p->common.client_dc.rect            = rect;

    /* Update all chlidren */
    temp_p = p->head.fc;
    while (temp_p != NULL)
    {
        in_win_move(temp_p, left, top);
        temp_p = temp_p->head.next;
    }     

    /* Invalidate window */
    if ( in_win_is_visual(p) )
        in_win_invalidate_below_area_abs(p, &(p->common.win_dc.rect));

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_resize_window(/* HWND  hwnd */ void *hwnd, int left, int top, int right, int bottom)
{
    int  ret = 0;
   
    gui_lock( );
    ret = in_win_resize(hwnd, left, top, right, bottom);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_set_focus(/* HWND hwnd */ void *hwnd)
{
    HWND  p                 = hwnd;
    HWND  last_focus_hwnd   = NULL;
    HWND  main_p            = NULL;
    HWND  temp_p            = NULL;


    if ( p == NULL )
        return  -1;
    if ( p == lhfocu )
        return  -1;

    if ( in_win_is_visual(p) < 1 )
        return  -1;
    /* Optimize ? */
    if ( in_win_has(p) < 1 )
        return  -1;


    /* Dispatch all message */
    in_message_dispatch_all();


    last_focus_hwnd = lhfocu;

    lhfocu          = p;
    lhdefa          = p;
    #ifdef   _LG_MTJT_
    lhmtjt          = p;
    #endif


    main_p = p;


    if ( p == lhlist )
        goto  IN_WIN_SET_FOCUS_LAST;


    if ( p == HWND_DESKTOP )
        goto  IN_WIN_SET_FOCUS_LAST;


    /* It is a main window ? 1 */
    if ( (p->head.parent) == HWND_DESKTOP )
    {
        main_p = p;
        goto  IN_WIN_SET_FOCUS_EXCHANGE;
    }

    /* Looking for its main window 2 */
    temp_p = p; 
    while ( temp_p != NULL )
    {
        if ( (temp_p->head.parent) == HWND_DESKTOP )
        {
            main_p = temp_p;
            goto  IN_WIN_SET_FOCUS_EXCHANGE;
        }
        temp_p = temp_p->head.parent;
    }
        
    return  -1;

/* Exchange with lhlist */
IN_WIN_SET_FOCUS_EXCHANGE:
    if ( main_p == lhlist )
        goto  IN_WIN_SET_FOCUS_LAST;

    /* Prev */
    temp_p = main_p->head.prev;
    temp_p->head.next = main_p->head.next;

    /* Next */
    temp_p = main_p->head.next;
    if ( temp_p != NULL )
        temp_p->head.prev = main_p->head.prev;

    /* Main */
    main_p->head.prev = lhlist;
    main_p->head.next = NULL;

    /* Old lhlist */
    lhlist->head.next = main_p;

    /* Set new lhlist */
    lhlist = main_p;

    /* Reapint main window */
    in_win_message_send_ext(lhlist, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);


IN_WIN_SET_FOCUS_LAST:
    in_win_message_send_ext(last_focus_hwnd, MSG_LOST_FOCUS, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
    in_win_message_send_ext(p, MSG_GET_FOCUS, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_set_focus(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_set_focus(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_is_focus(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;


    if  ( p == NULL )
        return  -1;

    if ( p == lhfocu )
        return  1;

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_is_focus(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_is_focus(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

HWND in_win_get_focus(void)
{
    return  lhfocu;
}

#ifndef  _LG_ALONE_VERSION_
HWND  win_get_focus(void)
{
    HWND  hwnd = HWND_DESKTOP;

    gui_lock( );
    hwnd = in_win_get_focus();
    gui_unlock( );

    return  hwnd;
}
#endif

/* Last default window and current default hwnd */
int  in_win_set_default(/* HWND hwnd */ void *hwnd)
{
    HWND   p                 = hwnd;
    HWND   last_default_hwnd = NULL;


    if (p == NULL)
        return  -1;
    if (in_win_is_visual(p) < 1)
        return  -1;
    if ( in_win_has(p) < 1 )
        return  -1;


    last_default_hwnd = lhdefa;
    lhdefa     = p;
    if ( last_default_hwnd == lhdefa )
        return  -1;


    in_win_invalidate_area_abs(&(last_default_hwnd->common.win_dc.rect));
    in_win_invalidate_area_abs(&(p->common.win_dc.rect));

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_set_default(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_set_default(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_win_is_default(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;


    if  ( p == NULL )
        return  -1;

    if ( p == lhdefa )
        return  1;

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_is_default(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_is_default(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

HWND in_win_get_default(void)
{
    return  lhdefa;
}

#ifndef  _LG_ALONE_VERSION_
HWND  win_get_default(void)
{
    HWND  hwnd = HWND_DESKTOP;

    gui_lock( );
    hwnd = in_win_get_default();
    gui_unlock( );

    return  hwnd;
}
#endif

/* Enable default or focus */
int  in_win_enable_default_focus(/* HWND hwnd */ void *hwnd)
{
    HWND   p = hwnd;


    if ( p == NULL )
        return  -1;

    p->common.no_focus_flag = 0;
    /* ?? 20170627 */

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_enable_default_focus(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_enable_default_focus(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

/* Disable default or focus */
int  in_win_disable_default_focus(/* HWND hwnd */ void *hwnd)
{
    HWND   p = hwnd;


    if ( p == NULL )
        return  -1;

    p->common.no_focus_flag = 1;
    /* ?? 20170627 */

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_disable_default_focus(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_disable_default_focus(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

/* Enable erase background */
int  in_win_enable_erase_back(/* HWND hwnd */ void *hwnd)
{
    HWND   p = hwnd;


    if ( p == NULL )
        return  -1;

    p->common.no_erase_back_flag = 0;
    /* ?? 20170627 */

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_enable_erase_back(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_enable_erase_back(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

/* Disable erase back */
int  in_win_disable_erase_back(/* HWND hwnd */ void *hwnd)
{
    HWND   p = hwnd;


    if ( p == NULL )
        return  -1;

    p->common.no_erase_back_flag = 1;
    /* ?? 20170627 */

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_disable_erase_back(/* HWND hwnd */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_disable_erase_back(hwnd);
    gui_unlock( );

    return  ret;
}
#endif


/* Only for main wind and set it to focus window */
int  in_win_bring_to_top(/* HWND hwnd */ const void *hwnd)
{
    HWND  p   = (HWND)hwnd;
    int   ret = 0;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;
    if ( (p->head.parent) != HWND_DESKTOP )
        return  -1;
    if ( p == lhlist )
        return  1;


    ret = in_win_set_focus(p);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  win_bring_to_top(/* HWND hwnd */ const void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_bring_to_top(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

static  HWND  in_win_get_hwnd_by_id_recursion(/* HWND hwnd */ const void *hwnd, unsigned int id)
{
    HWND  p     = (HWND)hwnd;
    HWND  tmp_p = NULL;
    HWND  ret_p = NULL;


    if ( p == NULL )
        return  NULL;

    if ( (p->common.id) == id  )
        return  p;

    tmp_p = p->head.fc;
    while ( tmp_p != NULL )
    {
        if ( (tmp_p->common.id) == id )
            return  tmp_p;

        if ( (tmp_p->head.fc) != NULL )
        {
            ret_p = in_win_get_hwnd_by_id_recursion(tmp_p->head.fc, id);
            if ( ret_p != NULL )
                return  ret_p;
        }
        tmp_p = tmp_p->head.next;
    }

    return  NULL;
}

HWND  in_win_get_hwnd_by_id(/* HWND hwnd */ const void *hwnd, unsigned int id)
{
    HWND  p     = (HWND)hwnd;
    HWND  tmp_p = NULL;
    HWND  ret_p = NULL;


    if ( p == NULL )
        p = HWND_DESKTOP;

    if ( p != HWND_DESKTOP ) 
        return  in_win_get_hwnd_by_id_recursion(p, id);


    tmp_p = lhlist;
    while ( tmp_p != NULL )
    {
        ret_p = in_win_get_hwnd_by_id_recursion(tmp_p, id);
        if ( ret_p != NULL )
            return  ret_p;

        tmp_p = tmp_p->head.prev;
    }

    return  NULL;
}

#ifndef  _LG_ALONE_VERSION_
HWND  win_get_hwnd_by_id(/* HWND hwnd */ const void *hwnd, unsigned int id)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_win_get_hwnd_by_id(hwnd, id);
    gui_unlock( );

    return  p;
}
#endif  /* _LG_ALONE_VERSION_ */


/* Maybe change levle-1 window */
int  in_win_set_default_by_key(int key)
{
    HWND  last_default_hwnd = lhdefa;
    HWND  tmp_p = NULL;


    if ( (key != GUI_KEY_TAB)&&(key != GUI_KEY_BACK_TAB) )
        return  -1;


    if ( (lhdefa == NULL) || (lhdefa == HWND_DESKTOP) )
    {
        if ( lhlist == NULL )
            return  -1;

        /* lhlist  */
        if ( key == GUI_KEY_BACK_TAB )
            tmp_p = lhlist->head.lc;
        else
            tmp_p = lhlist->head.fc;

        KEY_TAB_MOVE_STEP1:
        /* Has not any children */ 
        if ( tmp_p == NULL )
        {
            lhdefa = lhlist;        
            if ( lhdefa == NULL )
                return  -1;

            goto  KEY_DEFAULT_MOVE_OK;
        }

        /* Move the first child(it could get focus) */
        while ( tmp_p != NULL )
        {
            if ( (((tmp_p->common.style)&VISUAL_STYLE)==VISUAL_STYLE)&&(((tmp_p->common.style)&ENABLE_STYLE)==ENABLE_STYLE )&&(tmp_p->common.no_focus_flag < 1) )
            {
                lhdefa = tmp_p;        
                goto  KEY_DEFAULT_MOVE_OK;
            }

            if ( key == GUI_KEY_BACK_TAB )
                tmp_p = tmp_p->head.prev;
            else
                tmp_p = tmp_p->head.next;
        }

        /* Has not any child (it could get focus) */
        lhdefa = lhlist;        
        if ( lhdefa == NULL )
            return  -1;

        goto  KEY_DEFAULT_MOVE_OK;
    }

    /* Move to the next one */
    if ( (lhdefa->head.parent) == HWND_DESKTOP )
    {
        /* Move to close, min, max button */

        /* */

        if ( key == GUI_KEY_BACK_TAB )
            tmp_p = lhdefa->head.lc;
        else
            tmp_p = lhdefa->head.fc;

        /* 20170620 add. Have not any child window. */
        if ( tmp_p != NULL)
            goto  KEY_TAB_MOVE_STEP1;
    }

    tmp_p = NULL;
    if ( lhdefa != NULL )
    {
        if ( key == GUI_KEY_BACK_TAB )
            tmp_p = lhdefa->head.prev;
        else
            tmp_p = lhdefa->head.next;
    }

    while ( tmp_p != NULL )
    {
        if ( (((tmp_p->common.style)&VISUAL_STYLE)==VISUAL_STYLE)&&(((tmp_p->common.style)&ENABLE_STYLE)==ENABLE_STYLE )&&(tmp_p->common.no_focus_flag < 1) )
        {
            lhdefa = tmp_p;
            goto  KEY_DEFAULT_MOVE_OK;
        }

        if ( key == GUI_KEY_BACK_TAB )
            tmp_p = tmp_p->head.prev;
        else
            tmp_p = tmp_p->head.next;
    }

    /* Has not any child (it could get focus) */
    lhdefa = lhlist;        
    if ( lhdefa == NULL )
        return  -1;

KEY_DEFAULT_MOVE_OK:
    if ( last_default_hwnd == lhdefa )
        return  1;

    in_win_message_send_ext(last_default_hwnd, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
    in_win_message_send_ext(lhdefa, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#endif  /* _LG_WINDOW_ */
