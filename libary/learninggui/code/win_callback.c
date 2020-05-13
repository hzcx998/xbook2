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

#include  <keyboard.h>

#include  <rect_ops.h>

#include  <win_tools.h>
#include  <win_interface.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_callback.h>

#include  "win_interface_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
int  in_callback_to_hwnd_message(/* GUI_MESSAGE */ void *msg)
{
    GUI_MESSAGE  *p_msg = (GUI_MESSAGE *)msg;
    HWND          p     = NULL;
    int           ret   = 1;


    if ( p_msg == NULL )
        return  -1;
    p = p_msg->to_hwnd;
    if ( p == NULL )
        return  -1;


    ret = 1;
    /* Repeation ?? */
    if ( ((p_msg->callback_flag)&HWND_APP_CALLBACK) == HWND_APP_CALLBACK )
    {
        if ( ((p->common.is_app_callback) > 0)&&((p->common.app_callback) != NULL) )
        {
            gui_unlock();
            ret = (*(p->common.app_callback))(p_msg);
            gui_lock();
        }
    }
    if ( ret < 1 )
        return  1;


    if ( ((p_msg->callback_flag)&HWND_IN_CALLBACK) == HWND_IN_CALLBACK )
    {
        if ( ((p->common.is_in_callback) > 0)&&((p->common.in_callback) != NULL ) )
            ret = (*(p->common.in_callback))(p_msg);
    }
 
    return  ret;
}
 
#ifdef   _LG_MTJT_
static  HWND  in_get_hwnd_mtjt_in_recursion(/* HWND hwnd */ void *hwnd, /* GUI_MESSAGE *msg */ void *msg)
{
    HWND        p      = (HWND)hwnd;
    HWND        temp_p = NULL;

    GUI_RECT    rect;
    int         x      = -1;
    int         y      = -1;


    if ( p == NULL )
        return  HWND_DESKTOP;
    if ( msg == NULL )
        return  HWND_DESKTOP;


    x = MESSAGE_GET_MTJT_X(msg);
    y = MESSAGE_GET_MTJT_Y(msg);

    while (p != NULL)
    {
        if ( (p->head.lc) == NULL )
            goto  GET_HWND_MTJT_WIDGET;


        if ( lhfocu == NULL )
            goto  GET_HWND_MTJT_IN_RECURSION;
        if ( (lhfocu->head.parent) != ((p->head.fc)->head.parent) )
            goto  GET_HWND_MTJT_IN_RECURSION;

        rect = lhfocu->common.win_dc.rect;
        if ( (x>=rect.left)&&(x<=rect.right)&&(y>=rect.top)&&(y<=rect.bottom) )
            return  lhfocu;


        GET_HWND_MTJT_IN_RECURSION:
        temp_p = in_get_hwnd_mtjt_in_recursion(p->head.lc, msg);
        if ( temp_p != HWND_DESKTOP )
            return  temp_p;


        GET_HWND_MTJT_WIDGET:
        if ( (p->common.no_focus_flag) > 0 )
            goto  GET_HWND_MTJT_CONTINUE;

        if ( in_win_is_visual(p) <  1 )
            goto  GET_HWND_MTJT_CONTINUE;

        if ( in_win_is_enable(p) > 0 )
            goto  GET_HWND_MTJT_RECT;

        GET_HWND_MTJT_CONTINUE:
        p = p->head.prev;
        continue;


        GET_HWND_MTJT_RECT:
        rect = p->common.win_dc.rect;
        if ( (x>=rect.left)&&(x<=rect.right)&&(y>=rect.top)&&(y<=rect.bottom) )
            return  p;

        p = p->head.prev;
    }

    return  HWND_DESKTOP;
}
#endif  /*  _LG_MTJT_ */


 /* Default window message routine  */
int  in_message_window_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    int            ret  = 1;
    #ifdef   _LG_MTJT_
    HWND           p    = NULL;
    #endif
    #ifdef   _LG_MTJT_
    #ifdef  _LG_WINDOW_BAR_
    GUI_WINBAR    *wbar = NULL;
    #endif
    #endif
    #ifdef  _LG_MTJT_
    GUI_RECT       rect;
    #endif
    #ifdef  _LG_KEYBOARD_
    GUI_MESSAGE    tmsg;
    #endif
    #ifdef  _LG_MTJT_
    int             x   = 0;
    int             y   = 0;
    #endif
    #ifdef  _LG_KEYBOARD_
    int            key_value = 0;
    #endif



    if ( msg == NULL )
        return  -1;


    #ifdef   _LG_MTJT_
    p = HWND_DESKTOP;
    #endif

    #ifdef  _LG_MTJT_
    ret = MESSAGE_IS_MTJT(msg);
    if ( ret > 0 )
    {
        p  = in_get_hwnd_mtjt_in_recursion(lhlist, msg);
        if ( p == NULL )
            return  -1;
    
        ((GUI_MESSAGE *)msg)->to_hwnd = p;

        x = MESSAGE_GET_MTJT_X(msg);
        y = MESSAGE_GET_MTJT_Y(msg);
    }
    #endif

    switch (MESSAGE_GET_ID(msg))
    {
        #ifdef   _LG_MTJT_
        case  MSG_MTJT_LBUTTON_UP:
            #ifdef  _LG_WINDOW_BAR_
            if ( in_win_is_visual(p) && IS_WINBAR_WIDGET(&(p->common)))
            {
                wbar = in_win_get_window_bar(p);
                if ( wbar != NULL )
                {
                    rect = wbar->close_rect;
                    if ( (x >= rect.left)&&(x <= rect.right)&&(y >= rect.top)&&(y <= rect.bottom))
                    {
                        /* in_win_close(p); */
                        in_make_callback_message(p, MSG_CLOSE, HWND_IN_CALLBACK | HWND_APP_CALLBACK);
                        return  0;
                    }

                    rect = wbar->max_rect;
                    if ( (x >= rect.left)&&(x <= rect.right)&&(y >= rect.top)&&(y <= rect.bottom))
                    {
                        if ( (wbar->status) == WINDOW_NORMAL_STATUS )
                        {
                            in_make_callback_message(p, MSG_MAXIZE, HWND_IN_CALLBACK | HWND_APP_CALLBACK);
                        } else {
                            in_make_callback_message(p, MSG_NORMAL, HWND_IN_CALLBACK | HWND_APP_CALLBACK);
                        }
                        return  0;
                    }

                    rect = wbar->min_rect;
                    if ( (x >= rect.left)&&(x <= rect.right)&&(y >= rect.top)&&(y <= rect.bottom))
                    {
                        in_make_callback_message(p, MSG_HIDE, HWND_IN_CALLBACK | HWND_APP_CALLBACK);
                        return  0;
                    }
                }
            }
            #endif  /* _LG_WINDOW_BAR_ */

            /* Sequence ?? */
            in_win_set_focus(p);
            break;
        #endif  /* _LG_MTJT_ */


        #ifdef  _LG_KEYBOARD_
        case  MSG_KEY_DOWN:
            key_value = MESSAGE_GET_KEY_VALUE(msg);
            if ( (key_value == GUI_KEY_PAGE_DOWN) || (key_value == GUI_KEY_PAGE_UP) )
            {
                /* HWND_DESKTOP->head.next will be changed */
                in_win_set_focus(HWND_DESKTOP->head.next);
                return  1;
            }

            if ( (key_value == GUI_KEY_TAB) || (key_value == GUI_KEY_BACK_TAB) )
            {
                in_win_set_default_by_key(key_value);
                return  1;
            }

            if ( key_value == GUI_KEY_ENTER )
            {
                /* Press enter, lhfocu or lhdefa is changed ?? */
                memset((void *)(&tmsg), 0, sizeof(GUI_MESSAGE));
                tmsg.id             = MSG_KEY_DOWN;
                tmsg.data0.value    = GUI_KEY_ENTER;
                tmsg.to_hwnd        = lhfocu;
                tmsg.callback_flag  = HWND_IN_CALLBACK | HWND_APP_CALLBACK;

                if ( lhdefa == lhfocu )
                {
                    in_callback_to_hwnd_message(&tmsg);

                    in_win_set_default_by_key(GUI_KEY_TAB);
                }

                in_win_set_focus(lhdefa);

                tmsg.to_hwnd        = lhdefa;
                in_callback_to_hwnd_message(&tmsg);
 
                return  1;
            }
            break;
        #endif  /* _LG_KEYBOARD_ */

        default:
            break;
    }

    ret = in_callback_to_hwnd_message(msg);
    if ( ret < 1 )
        return  -1;

    return  1;
}
#endif  /* _LG_WINDOW_ */
