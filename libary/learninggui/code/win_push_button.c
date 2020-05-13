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
#include  <cursor.h>

#include  <win_tools.h>
#include  <win_callback.h>
#include  <win_invalidate.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_widget.h>
#include  <win_push_button.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_PUSH_BUTTON_WIDGET_

/* PushButton internal callback */
static  int  in_push_button_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND       p;
    HDC        hdc;
    GUI_RECT   rect;
    GUI_COLOR  old_color;

 
    if ( msg == NULL )
        return  -1;

    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch ( MESSAGE_GET_ID(msg) )
    {
        case  MSG_PAINT:
            if ( (GET_IN_GUI_PUSH_BUTTON(p)->is_ghost_flag) > 0 )
                return  0;

            if ( in_win_is_visual(p) < 1 )
                break;

            in_win_set_current_color_group(p);

            /* paint backgroud */
            in_paint_widget_back(p);

            /* paint border */
            if ( IS_BORDER_WIDGET(&(p->common)) > 0 )
            {
                if ( IS_BORDER_3D_WIDGET(&(p->common)) > 0 )
                    in_paint_3d_up_border(p, &(p->common.win_dc.rect));
                else
                    in_paint_widget_border(p);
            }

            /* paint text */
            /* Unicode text ?? */
            if (strlen((GET_IN_GUI_PUSH_BUTTON(p)->text)) < 1)
            {
                in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_APP_CALLBACK);
                return  0;
            }

            hdc = in_hdc_get_client(p);
            if ( hdc == NULL )
                break;

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
            #endif

            rect.left   = 0;
            rect.top    = 0;
            rect.right  = GUI_RECTW(&(hdc->rect))-1;
            rect.bottom = GUI_RECTH(&(hdc->rect))-1;
     
            old_color = in_hdc_get_back_color(hdc);

            if ( (p == lhdefa)&&(p != lhfocu) ) 
                in_hdc_set_back_color(hdc, DEFAULT_WINDOW_BCOLOR);

            in_text_out_rect(hdc, &rect, GET_IN_GUI_PUSH_BUTTON(p)->text, -1, LG_TA_CENTER);

            in_hdc_set_back_color(hdc, old_color);
            in_hdc_release_win(p, hdc);

            p->common.invalidate_flag = 0;

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_refresh( );
            #endif
 
            in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_APP_CALLBACK);
            return  0;

        default:
            break;
    }

    return  in_common_widget_callback(msg);
}

HWND in_push_button_create(HWND parent, void *gui_common_widget, void *gui_push_button)
{
    HWND                 p  = NULL;
    GUI_PUSH_BUTTON     *push_button = (GUI_PUSH_BUTTON *)gui_push_button;
    IN_GUI_PUSH_BUTTON   in_push_button;
    unsigned int         size = 0;
    int                  ret;
   

    /* Adjust parent */
    if ( parent == NULL )
        parent = HWND_DESKTOP;
    if ( in_win_has(parent) < 0 )
        return  NULL;


    /* 
     * set hwnd common 
     */

    /* set hwnd common1 */
    /* ID, style, ext_style, default win_dc */
    /* Border ?? */
    /* set ltcomm and ltdc value */
    ret = in_set_hwnd_common1(parent, PUSH_BUTTON_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;


    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = PBTN_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = PBTN_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = PBTN_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = PBTN_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = PBTN_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = PBTN_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc                         = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = PBTN_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = PBTN_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = PBTN_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = PBTN_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = PBTN_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = PBTN_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = PBTN_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = PBTN_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = PBTN_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = PBTN_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = PBTN_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = PBTN_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc                    = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.in_callback = in_push_button_callback;


    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_PUSH_BUTTON);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */
    memset(&in_push_button, 0, sizeof(in_push_button));
    if ( push_button != NULL )
        in_push_button = *push_button;
    memcpy(p->ext, &in_push_button, sizeof(IN_GUI_PUSH_BUTTON));

    /* 
     * add hwnd and post message.
     */
    ret = in_deal_add_hwnd(gui_common_widget, p, 0);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  push_button_create(HWND parent, void *gui_common_widget, void *gui_push_button)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_push_button_create(parent, gui_common_widget, gui_push_button);
    gui_unlock( );

    return  p;
}
#endif

int  in_push_button_set_text(HWND hwnd, TCHAR  *text, unsigned int  text_len)
{
    HWND   p = hwnd;
    unsigned int  copy_len = 0;


    if ( p == NULL )
        return  -1;
    if ( text == NULL )
        return  -1;
    if ( text_len < 1 )
        return  -1;
    if ( (p->common.type) != PUSH_BUTTON_WIDGET_TYPE )
        return  -1;


    copy_len = (text_len > MAX_PUSH_BUTTON_TEXT_LEN ? MAX_PUSH_BUTTON_TEXT_LEN : text_len);
      
    memset( GET_IN_GUI_PUSH_BUTTON(p)->text, 0, MAX_PUSH_BUTTON_TEXT_LEN);
    memcpy( GET_IN_GUI_PUSH_BUTTON(p)->text, text, copy_len);

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  push_button_set_text(HWND hwnd, TCHAR  *text, unsigned int  text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_push_button_set_text(hwnd, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif

int  in_push_button_get_text(HWND hwnd, TCHAR  *text, unsigned int  *text_len)
{
    HWND   p = hwnd;
    unsigned int  copy_len = 0;


    if ( p == NULL )
        return  -1;
    if ( text == NULL )
        return  -1;
    if ( *text_len < 1 )
        return  -1;
    if ( (p->common.type) != PUSH_BUTTON_WIDGET_TYPE )
        return  -1;

    copy_len = (*text_len > MAX_PUSH_BUTTON_TEXT_LEN ? MAX_PUSH_BUTTON_TEXT_LEN : *text_len);
    memcpy(text, GET_IN_GUI_PUSH_BUTTON(p)->text, copy_len);
    *text_len = copy_len;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  push_button_get_text(HWND hwnd, TCHAR  *text, unsigned int  *text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_push_button_get_text(hwnd, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif

int  in_push_button_set_ghost(HWND hwnd)
{
    HWND   p = hwnd;


    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != PUSH_BUTTON_WIDGET_TYPE )
        return  -1;

    GET_IN_GUI_PUSH_BUTTON(p)->is_ghost_flag = 1;

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  push_button_set_ghost(HWND hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_push_button_set_ghost(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

int  in_push_button_is_ghost(HWND hwnd)
{
    HWND   p = hwnd;
    
    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != PUSH_BUTTON_WIDGET_TYPE )
        return  -1;

    if ( (GET_IN_GUI_PUSH_BUTTON(p)->is_ghost_flag) > 0 )
        return  1;

    return  0;
}

#ifndef  _LG_ALONE_VERSION_
int  push_button_is_ghost(HWND hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_push_button_is_ghost(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_PUSH_BUTTON_WIDGET_ */
#endif  /* _LG_WINDOW_ */
