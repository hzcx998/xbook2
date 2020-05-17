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

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#include  <lock.h>
#include  <cursor.h>

#include  <d2_rect.h>

#include  <win_tools.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_widget.h>
#include  <win_progress_bar.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_PROGRESS_BAR_WIDGET_

/* ProgressBar internal callback */
static  int  in_progress_bar_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND                 p;
    HDC                  hdc;
    GUI_RECT             rect;
    IN_GUI_PROGRESS_BAR  in_pgbar;
    GUI_COLOR            old_color;

    UINT64               value  = 0;
    UINT64               offset = 0;
    UINT64               tmp    = 0;
     int                 i      = 0;

    /* char or unsigned char ? */
    char                 tbuf1[32] = "";
    char                 tbuf2[32] = "";



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

            hdc = in_hdc_get_client(p);
            if ( hdc == NULL )
                break;

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
            #endif

            in_pgbar = *(GET_IN_GUI_PROGRESS_BAR(p));

            /* Scaled percent value */
            value  = in_pgbar.current_value - in_pgbar.min_value;
            for (i = 0; i < (in_pgbar.decimal_digits + 3); i++)
                value *= 10;
            value  /= (in_pgbar.max_value - in_pgbar.min_value);

            /* Current_value point offset */
            if (((p->common.ext_style)&PGBAR_VBAR_STYLE) == PGBAR_VBAR_STYLE)
                offset = value*GUI_RECTH(&(hdc->rect));
            else
                offset = value*GUI_RECTW(&(hdc->rect));

            for (i = 0; i < (in_pgbar.decimal_digits + 3); i++)
                offset /= 10;


            /* Notice: if PGVBAR ext style, offset direction: down. In fact, need up */
            /* Adjust offset */
            if (((p->common.ext_style)&PGBAR_VBAR_STYLE) == PGBAR_VBAR_STYLE)
            {
                tmp    = GUI_RECTH(&(hdc->rect)) - offset;
                offset = tmp;
            }

            /* Adjust value */
            value += 5;

            /* Prepare rect value */
            rect.left   = 0;
            rect.right  = GUI_RECTW(&(hdc->rect))-1;
            rect.top    = 0;
            rect.bottom = GUI_RECTH(&(hdc->rect))-1;

            /* Draw blank rect */
            if (((p->common.ext_style)&PGBAR_VBAR_STYLE) == PGBAR_VBAR_STYLE)
            {
                rect.top     = 0;
                rect.bottom  = offset - 1; 
            } else {
                rect.left   = offset + 1;
                rect.right  = GUI_RECTW(&(hdc->rect))-1;  
            }

            /* Draw fill rect */
            if (((p->common.ext_style)&PGBAR_VBAR_STYLE) == PGBAR_VBAR_STYLE)
            {
                rect.top    = offset ;
                rect.bottom = GUI_RECTH(&(hdc->rect))-1;
            } else {
                rect.left   = 0;
                rect.right  = offset;
            }

            old_color = in_hdc_get_back_color(hdc); 
            in_hdc_set_back_color(hdc, GUI_WHITE);

            #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
            if ( (p->common.bimage_flag) > 0 )
                in_rect_frame(hdc, rect.left, rect.top, rect.right, rect.bottom);
            else
            #endif
                in_rect_fill(hdc, rect.left, rect.top, rect.right, rect.bottom);

            in_hdc_set_back_color(hdc, old_color);


            /* Draw text */
            if ( (in_pgbar.is_display_text) < 1 )
                goto  PGBAR_DRAW_END;

            rect.left   = 0;
            rect.right  = GUI_RECTW(&(hdc->rect))-1;
            rect.top    = 0;
            rect.bottom = GUI_RECTH(&(hdc->rect))-1;

            memset(tbuf1, 0, sizeof(tbuf1));

            if ( (in_pgbar.display_style) == PGBAR_DISPLAY_PERCENT_STYLE )
                goto  PGBAR_PERCENT_STYLE;

            sprintf(tbuf1, "%d", in_pgbar.current_value);
            goto  PGBAR_TEXT_OUT_RECT;


            PGBAR_PERCENT_STYLE:
            /* Integer part */
            tmp = value;
            for( i = 0; i < (in_pgbar.decimal_digits + 1) ; i++ )
                tmp  /= 10;

            sprintf(tbuf1, "%lld", tmp);


            /* Decimal part */
            if ( in_pgbar.decimal_digits == 0 )
                goto  PGBAR_BUF_END;

            strcat(tbuf1, ".");

            offset = 1;
            for( i = 0; i < (in_pgbar.decimal_digits + 1); i++ )
                offset *= 10;
            
            tmp  = value;
            tmp %= offset;
            tmp /= 10;

            memset(tbuf2, 0, sizeof(tbuf2));
            sprintf(tbuf2, "%lld", tmp);

            /* Append '0' in some case */
            for( i = 0; i < (in_pgbar.decimal_digits); i++ )
            {
                if ( strlen(tbuf2) >= (in_pgbar.decimal_digits) )
                    break;

                strcat(tbuf2, "0");
            }

            strcat(tbuf1, tbuf2);

            PGBAR_BUF_END:
            strcat(tbuf1,"%");

            PGBAR_TEXT_OUT_RECT:
            in_text_out_rect(hdc, &rect, tbuf1, -1, LG_TA_CENTER);


            /* Draw end */
            PGBAR_DRAW_END:
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

HWND in_progress_bar_create(HWND parent, void *gui_common_widget, void *gui_progress_bar)
{
    HWND                p  = NULL;
    GUI_PROGRESS_BAR   *progress_bar  = (GUI_PROGRESS_BAR *)gui_progress_bar;
    IN_GUI_PROGRESS_BAR in_progress_bar;
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
    ret = in_set_hwnd_common1(parent, PROGRESS_BAR_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;


    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = PGBAR_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = PGBAR_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = PGBAR_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = PGBAR_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = PGBAR_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = PGBAR_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc                         = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = PGBAR_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = PGBAR_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = PGBAR_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = PGBAR_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = PGBAR_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = PGBAR_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = PGBAR_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = PGBAR_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = PGBAR_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = PGBAR_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = PGBAR_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = PGBAR_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc                    = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.no_focus_flag = 1;
    ltcomm.in_callback   = in_progress_bar_callback;


    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_PROGRESS_BAR);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */
    memset(&in_progress_bar, 0, sizeof(in_progress_bar));
    if ( progress_bar != NULL )
    {
        in_progress_bar.min_value            = progress_bar->min_value; 
        in_progress_bar.max_value            = progress_bar->max_value;
        in_progress_bar.current_value        = progress_bar->current_value;
        in_progress_bar.is_display_text      = progress_bar->is_display_text;
        in_progress_bar.display_style        = progress_bar->display_style;
        in_progress_bar.decimal_digits       = progress_bar->decimal_digits;
        if ( (in_progress_bar.decimal_digits) > PGBAR_MAX_DECIMAL_DIGITS )
           in_progress_bar.decimal_digits   = PGBAR_MAX_DECIMAL_DIGITS;
    }

    memcpy(p->ext, &in_progress_bar, sizeof(IN_GUI_PROGRESS_BAR));


    /* 
     * add hwnd and post message.
     */
    ret = in_deal_add_hwnd(gui_common_widget, p, 0);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  progress_bar_create(HWND parent, void *gui_common_widget, void *gui_progress_bar)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_progress_bar_create(parent, gui_common_widget, gui_progress_bar);
    gui_unlock( );

    return  p;
}
#endif

int  in_progress_bar_get_value(HWND hwnd, int *value)
{
    HWND   p = hwnd;


    if ( value == NULL )
        return  -1;
    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != PROGRESS_BAR_WIDGET_TYPE )
        return  -1;

    *value = GET_IN_GUI_PROGRESS_BAR(p)->current_value;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  progress_bar_get_value(HWND hwnd, int *value)
{
    int  ret = 0;

    gui_lock( );
    ret = in_progress_bar_get_value(hwnd, value);
    gui_unlock( );

    return  ret;
}
#endif

int  in_progress_bar_get_percent(HWND hwnd, unsigned int *percent)
{
    HWND                 p     = hwnd;
    IN_GUI_PROGRESS_BAR  in_pgbar;
    UINT64               value = 0;
    int                  i     = 0;


    if ( percent == NULL )
        return  -1;
    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != PROGRESS_BAR_WIDGET_TYPE )
        return  -1;


    in_pgbar = *(GET_IN_GUI_PROGRESS_BAR(p));
    value    = in_pgbar.current_value - in_pgbar.min_value;
    for (i = 0; i < (in_pgbar.decimal_digits + 3) ; i++)
         value *= 10;

    value  /= (in_pgbar.max_value - in_pgbar.min_value);
    value  += 5;

    *percent = (value/10);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  progress_bar_get_percent(HWND hwnd, unsigned int *percent)
{
    int  ret = 0;

    gui_lock( );
    ret = in_progress_bar_get_percent(hwnd, percent);
    gui_unlock( );

    return  ret;
}
#endif

int  in_progress_bar_set_value(HWND hwnd, int value)
{
    HWND   p = hwnd;


    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != PROGRESS_BAR_WIDGET_TYPE )
        return  -1;


    if ( value < (GET_IN_GUI_PROGRESS_BAR(p)->min_value) )
        return  -1;
    if ( value > (GET_IN_GUI_PROGRESS_BAR(p)->max_value) )
        return  -1;

        
    GET_IN_GUI_PROGRESS_BAR(p)->current_value = value;

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  progress_bar_set_value(HWND hwnd, int value)
{
    int  ret = 0;

    gui_lock( );
    ret = in_progress_bar_set_value(hwnd, value);
    gui_unlock( );

    return  ret;
}
#endif
     
int  in_progress_bar_set_percent(HWND hwnd, unsigned int percent)
{
    HWND    p      = hwnd;
    UINT64  offset = 0;
    UINT64  tmp    = 0;
     int    i      = 0;


    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != PROGRESS_BAR_WIDGET_TYPE )
        return  -1;

            
    offset = percent*( GET_IN_GUI_PROGRESS_BAR(p)->max_value - GET_IN_GUI_PROGRESS_BAR(p)->min_value);
    for( i = 0; i < ((GET_IN_GUI_PROGRESS_BAR(p)->decimal_digits) + 2) ; i++ )
        offset /= 10;

    tmp = GET_IN_GUI_PROGRESS_BAR(p)->min_value + offset;
    if ( tmp < (GET_IN_GUI_PROGRESS_BAR(p)->min_value) )
        tmp = GET_IN_GUI_PROGRESS_BAR(p)->min_value;
    if ( tmp > (GET_IN_GUI_PROGRESS_BAR(p)->max_value) ) 
        tmp = GET_IN_GUI_PROGRESS_BAR(p)->max_value;


    GET_IN_GUI_PROGRESS_BAR(p)->current_value = (int)tmp;

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  progress_bar_set_percent(HWND hwnd, unsigned int percent)
{
    int  ret = 0;

    gui_lock( );
    ret  = in_progress_bar_set_percent(hwnd, percent);
    gui_unlock( );

    return  ret;
}
#endif
    
#endif  /* _LG_PROGRESS_BAR_WIDGET_ */
#endif  /* _LG_WINDOW_ */
