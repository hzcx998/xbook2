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

#include  <d2_rect.h>

#include  <win_tools.h>
#include  <win_callback.h>
#include  <win_invalidate.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_widget.h>
#include  <win_frame.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_FRAME_WIDGET_

/* Frame internal callback */
static  int  in_frame_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND         p = NULL;
    HDC          hdc;
    GUI_RECT     rect;
    int          i = 0;


    if ( msg == NULL )
        return  -1;
    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch ( MESSAGE_GET_ID(msg) )
    {
        case  MSG_PAINT:
            if ( in_win_is_visual(p) < 1 )
                break;

            in_win_set_current_color_group(p);

            /* Draw window border */
            if (IS_BORDER_WIDGET(&(p->common)) < 1)
                goto  DRAW_FRAME1;
          
            hdc = in_hdc_get_window(p);
            if ( hdc == NULL )
                break;

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
            #endif

            rect = hdc->rect;
            for (i = 0; i < (p->common.border_width); i++)
                in_rect_frame(hdc, i, i, GUI_RECTW(&rect)-1-i, GUI_RECTH(&rect)-1-i);
            in_hdc_release_win(p, hdc);

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_refresh( );
            #endif
 
      
            DRAW_FRAME1:
            /* Draw window bar */
            #ifdef  _LG_WINDOW_BAR_
            if (IS_WINBAR_WIDGET(&(p->common)) < 1)
                goto  DRAW_FRAME2;

            in_win_paint_window_bar(p);
            #endif

            DRAW_FRAME2:
            /* Draw window client rect */
            in_paint_widget_back(p);

            in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_APP_CALLBACK);
            break;

        default:
            break;
    }

    return  in_common_widget_callback(msg);
}

/* Create frame */
HWND in_frame_create(HWND parent, void *gui_common_widget, void *gui_frame)
{
    HWND                p      = NULL;
    GUI_FRAME          *frame  = (GUI_FRAME *)gui_frame;

    unsigned int        str_len;
    unsigned int        copy_len;
    unsigned int        size = 0;
    int                 ret;
   

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
    ret = in_set_hwnd_common1(parent, FRAME_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;


    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = FRAME_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = FRAME_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = FRAME_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = FRAME_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = FRAME_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = FRAME_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = FRAME_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = FRAME_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = FRAME_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = FRAME_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = FRAME_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = FRAME_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = FRAME_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = FRAME_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = FRAME_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = FRAME_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = FRAME_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = FRAME_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.in_callback = in_frame_callback;


    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_FRAME);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */

    /* Window bar */
    /* Bar rect and title rect */
    #ifdef  _LG_WINDOW_BAR_
    if (IS_WINBAR_WIDGET(&ltcomm) < 1)
        goto  WBAR_OK;

    /* Bar title */
    if ( frame != NULL )
    {
        str_len  = ((GUI_FRAME *)frame)->len;
        copy_len = str_len < MAX_WIN_TEXT_LEN ? str_len : MAX_WIN_TEXT_LEN;
        memcpy((void *)(ltwbar.text), ((GUI_FRAME *)frame)->text, copy_len);
    }

    *((GUI_WINBAR *)(p->ext)) = ltwbar;
    p->common.winbar = (GUI_WINBAR *)(p->ext);
    WBAR_OK:
    #endif


    /* 
     * add hwnd and post message.
     */
    /* ?? sizeof(IN_GUI_FRAME) */
    ret = in_deal_add_hwnd(gui_common_widget, p, 0);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  frame_create(HWND parent, void *gui_common_widget, void *gui_frame)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_frame_create(parent, gui_common_widget, gui_frame);
    gui_unlock( );

    return  p;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_FRAME_WIDGET_ */
#endif  /* _LG_WINDOW_ */
