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

#include  <keyboard.h>

#include  <d2_line.h>
#include  <d2_rect.h>

#include  <win_tools.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_widget.h>
#include  <win_slider_bar.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_SLIDER_BAR_WIDGET_

/* SliderBar internal callback */
static  int  in_slider_bar_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND               p           = NULL;
    HDC                hdc         = NULL;

    IN_GUI_SLIDER_BAR  *in_slbar   = NULL;

    GUI_RECT           rect;
    GUI_RECT           old_rect;
    GUI_COLOR          old_color;

    int                key_value = 0;

    UINT64             tmp1 = 0;
    UINT64             tmp2 = 0;

    unsigned int       i    = 0;
    unsigned int       j    = 0;

    #ifdef  _LG_MTJT_
    int                x    = 0;
    int                y    = 0;
    #endif

 
    if ( msg == NULL )
        return  -1;

    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

            
    in_slbar = GET_IN_GUI_SLIDER_BAR(p);

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

            old_rect = hdc->rect;

            /* Draw the whole background */
            rect.left   = 0;
            rect.top    = 0;
            rect.right  = GUI_RECTW(&(hdc->rect))-1;
            rect.bottom = GUI_RECTH(&(hdc->rect))-1;
      
            /* Draw ruler */
            old_color = in_hdc_get_back_color(hdc);
            in_hdc_set_back_color(hdc, GUI_BLACK);

            if (((p->common.ext_style)&SLBAR_VBAR_STYLE) == SLBAR_VBAR_STYLE)
                tmp1  = GUI_RECTH(&rect) - (in_slbar->tick_width);
            else
                tmp1  = GUI_RECTW(&rect) - (in_slbar->tick_width);

            for ( i = 0; i < 11; i++ )
            {
                for ( j = 0; j < 2; j++ )
                {
                    tmp2 = (i*tmp1)/10 + j + (in_slbar->tick_width)/2;

                    if (((p->common.ext_style)&SLBAR_VBAR_STYLE) == SLBAR_VBAR_STYLE)
                    {
                        tmp2 = GUI_RECTH(&rect) - tmp2;
                        if ( tmp2 > (GUI_RECTH(&rect) - 1) )
                            tmp2 = (GUI_RECTH(&rect) - 1);

                        in_line(hdc, 0, (int)tmp2, in_slbar->ruler_height, (int)tmp2);

                    } else {

                        in_line(hdc, (int)tmp2, 0, (int)tmp2, in_slbar->ruler_height);

                    }
                }
            }

            in_hdc_set_back_color(hdc, old_color);

            /* Draw the snap-slot */
            rect = old_rect;
                    
            if (((p->common.ext_style)&SLBAR_VBAR_STYLE) == SLBAR_VBAR_STYLE)
            {
                rect.left     += (in_slbar->ruler_height) + (GUI_RECTW(&rect) - (in_slbar->ruler_height) - (in_slbar->slot_height))/2;
                rect.top      += (in_slbar->tick_width)/2;
                rect.right    = rect.left + in_slbar->slot_height; 
                rect.bottom  -= (in_slbar->tick_width)/2;

            } else {
                rect.left    += (in_slbar->tick_width)/2;
                rect.top     += (in_slbar->ruler_height) + (GUI_RECTH(&rect) - (in_slbar->ruler_height) - (in_slbar->slot_height))/2;
                rect.right   -= (in_slbar->tick_width)/2;
                rect.bottom   = rect.top + in_slbar->slot_height; 
            }

            hdc->rect = rect;          

            old_color = in_hdc_get_back_color(hdc);
            in_hdc_set_back_color(hdc, GUI_BLACK);
            in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
            in_hdc_set_back_color(hdc, old_color);
  
            old_color = in_hdc_get_fore_color(hdc);
            in_hdc_set_fore_color(hdc, GUI_3D_DOWN_BORDER_FCOLOR);
            in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

            in_hdc_set_fore_color(hdc, GUI_3D_DOWN_MID_FCOLOR);
            in_line(hdc, 1, 1, GUI_RECTW(&rect)-2, 1);
            in_line(hdc, 1, 2, 1, GUI_RECTH(&rect)-2);

            in_hdc_set_fore_color(hdc, GUI_3D_DOWN_BORDER_FCOLOR);
            in_line(hdc, 1, 2, GUI_RECTW(&rect)-2, 2);
            in_line(hdc, 2, 2, 2, GUI_RECTH(&rect)-2);

            in_hdc_set_fore_color(hdc, GUI_3D_DOWN_IN_FCOLOR);
            in_line(hdc, GUI_RECTW(&rect)-2, 1, GUI_RECTW(&rect)-2, GUI_RECTH(&rect)-2);
            in_line(hdc, 1, GUI_RECTH(&rect)-2, GUI_RECTW(&rect)-2, GUI_RECTH(&rect)-2);

            in_hdc_set_fore_color(hdc, GUI_3D_DOWN_IN2_FCOLOR);
            in_line(hdc, GUI_RECTW(&rect)-3, 1, GUI_RECTW(&rect)-3, GUI_RECTH(&rect)-2);
            in_line(hdc, 1, GUI_RECTH(&rect)-3, GUI_RECTW(&rect)-2, GUI_RECTH(&rect)-3);

            in_hdc_set_fore_color(hdc, old_color);


            /* Draw tick mark */
            rect = old_rect;

            if (((p->common.ext_style)&SLBAR_VBAR_STYLE) == SLBAR_VBAR_STYLE)
            {
                tmp1    = (in_slbar->current_value - in_slbar->min_value)*(GUI_RECTH(&rect) - in_slbar->tick_width);
                tmp1   /= (in_slbar->max_value - in_slbar->min_value);

                rect.left    += in_slbar->ruler_height;
                rect.bottom  -= (int)tmp1;
                rect.right   -= 1; 
                rect.top      = rect.bottom - in_slbar->tick_width;
            } else {
                tmp1    = (in_slbar->current_value - in_slbar->min_value)*(GUI_RECTW(&rect) - in_slbar->tick_width);
                tmp1   /= (in_slbar->max_value - in_slbar->min_value);

                rect.left    += (int)tmp1;
                rect.top     += in_slbar->ruler_height;
                rect.right    = rect.left + in_slbar->tick_width;
                rect.bottom  -= 1; 
            }

            hdc->rect = rect;          
            in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

            old_color = in_hdc_get_fore_color(hdc);

            in_hdc_set_fore_color(hdc, GUI_3D_UP_BORDER_FCOLOR);
            in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

            in_hdc_set_fore_color(hdc, GUI_3D_UP_MID_FCOLOR);
            for ( i= 0; i < 2; i++)
                in_line(hdc, 1, 1+i, GUI_RECTW(&rect)-2, 1+i);
            for ( i= 0; i < 2; i++)
                in_line(hdc, 1+i, 2, 1+i, GUI_RECTH(&rect)-2);
    
            in_hdc_set_fore_color(hdc, GUI_3D_UP_IN_FCOLOR);
            for ( i= 0; i < 2; i++)
                in_line(hdc, 1, GUI_RECTH(&rect)-1-i, GUI_RECTW(&rect)-2, GUI_RECTH(&rect)-1-i);
            for ( i= 0; i < 2; i++)
                in_line(hdc, GUI_RECTW(&rect)-1-i, 2, GUI_RECTW(&rect)-1-i, GUI_RECTH(&rect)-1);
 
            in_hdc_set_fore_color(hdc, old_color);


            /* End paint */
            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            p->common.invalidate_flag = 0;

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_refresh( );
            #endif
 
            in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_APP_CALLBACK);
            break;


        case  MSG_KEY_DOWN:
            key_value = MESSAGE_GET_KEY_VALUE(msg);

            /* GUI_KEY_LEFT, GUI_KEY_DOWN */            
            if ( (key_value != GUI_KEY_LEFT)&&(key_value != GUI_KEY_DOWN) )
                goto  SLBAR_KEY_RIGHT_UP;

            tmp1 = GET_IN_GUI_SLIDER_BAR(p)->step_value;
            GET_IN_GUI_SLIDER_BAR(p)->current_value -= (int)tmp1;
        
            if ( (GET_IN_GUI_SLIDER_BAR(p)->current_value) < (GET_IN_GUI_SLIDER_BAR(p)->min_value) )
                GET_IN_GUI_SLIDER_BAR(p)->current_value = GET_IN_GUI_SLIDER_BAR(p)->min_value;

            goto  SLBAR_MSG_KEY_DOWN_OK;


            /* GUI_KEY_RIGHT, GUI_KEY_UP */            
            SLBAR_KEY_RIGHT_UP:
            if ( (key_value != GUI_KEY_RIGHT)&&(key_value != GUI_KEY_UP) )
                goto  SLBAR_KEY_HOME;

            tmp1 = GET_IN_GUI_SLIDER_BAR(p)->step_value;
            GET_IN_GUI_SLIDER_BAR(p)->current_value += (int)tmp1;
        
            if ( (GET_IN_GUI_SLIDER_BAR(p)->current_value) > (GET_IN_GUI_SLIDER_BAR(p)->max_value) )
                GET_IN_GUI_SLIDER_BAR(p)->current_value = GET_IN_GUI_SLIDER_BAR(p)->max_value;

            goto  SLBAR_MSG_KEY_DOWN_OK;


            /* GUI_KEY_HOME */
            SLBAR_KEY_HOME:
            if ( key_value != GUI_KEY_HOME )
                goto  SLBAR_KEY_END;
 
            GET_IN_GUI_SLIDER_BAR(p)->current_value = GET_IN_GUI_SLIDER_BAR(p)->min_value;
            goto  SLBAR_MSG_KEY_DOWN_OK;

            /* GUI_KEY_END */
            SLBAR_KEY_END:
            if ( key_value != GUI_KEY_END )
                goto  SLBAR_MSG_KEY_DOWN_OK;
 
            GET_IN_GUI_SLIDER_BAR(p)->current_value = GET_IN_GUI_SLIDER_BAR(p)->max_value;

            /* goto  SLBAR_MSG_KEY_DOWN_OK; */


            /* MSG_KEY_DOWN OK */
            SLBAR_MSG_KEY_DOWN_OK:
            in_win_message_send_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            in_win_message_send_ext(p, MSG_NOTIFY_VALUE_CHANGED, HWND_APP_CALLBACK);
            return  0;


        #ifdef   _LG_MTJT_
        case  MSG_MTJT_LBUTTON_UP:
            hdc = in_hdc_get_client(p);
            old_rect = hdc->rect;
            rect = old_rect;

            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
            {
                hdc->rect = old_rect;
                in_hdc_release_win(p, hdc);
                break;
            }


            /* tick_rect ? */


            /* slider_rect ? */
            /* SLBAR_LBUTTON_SLIDER_RECT: */
            rect  = in_slbar->slider_rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
            {
                hdc->rect = old_rect;
                in_hdc_release_win(p, hdc);
                break;
            }

            hdc->rect = rect;

            /* Calculate current_value */
            if (((p->common.ext_style)&SLBAR_VBAR_STYLE) == SLBAR_VBAR_STYLE)
                tmp1  = ((rect.bottom) - y)*(in_slbar->max_value - in_slbar->min_value);
             else
                tmp1  = (x - (rect.left))*(in_slbar->max_value - in_slbar->min_value);

            tmp1  *=10000;
            tmp1  += 5;

            if (((p->common.ext_style)&SLBAR_VBAR_STYLE) == SLBAR_VBAR_STYLE)
                tmp1 /= GUI_RECTH(&rect);
            else
                tmp1 /= GUI_RECTW(&rect);

            tmp1 /=10000;       
    
                
            in_slbar->current_value = in_slbar->min_value + (int)tmp1;

            if ( in_slbar->current_value < in_slbar->min_value )
                in_slbar->current_value = in_slbar->min_value;
            if ( in_slbar->current_value > in_slbar->max_value )
                in_slbar->current_value = in_slbar->max_value;


            /* SLBAR_LBUTTON_OK: */
            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            in_win_message_send_ext(p, MSG_PAINT, HWND_IN_CALLBACK);
            in_win_message_send_ext(p, MSG_NOTIFY_VALUE_CHANGED, HWND_APP_CALLBACK);
            return  0;
        #endif

        default:
            break;
    }

    return  in_common_widget_callback(msg);
}

HWND in_slider_bar_create(HWND parent, void *gui_common_widget, void *gui_slider_bar)
{
    HWND                p  = NULL;
    GUI_SLIDER_BAR     *slider_bar = (GUI_SLIDER_BAR *)gui_slider_bar;
    IN_GUI_SLIDER_BAR   in_slider_bar;

    GUI_RECT            rect;
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
    ret = in_set_hwnd_common1(parent, SLIDER_BAR_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;


    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = SLBAR_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = SLBAR_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = SLBAR_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = SLBAR_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = SLBAR_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = SLBAR_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc                         = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = SLBAR_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = SLBAR_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = SLBAR_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = SLBAR_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = SLBAR_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = SLBAR_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = SLBAR_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = SLBAR_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = SLBAR_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = SLBAR_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = SLBAR_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = SLBAR_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc                    = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.in_callback = in_slider_bar_callback;


    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_SLIDER_BAR);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */
    memset(&in_slider_bar, 0, sizeof(in_slider_bar));

    if ( slider_bar != NULL )
    {
        in_slider_bar.min_value        = slider_bar->min_value; 
        in_slider_bar.max_value        = slider_bar->max_value;
        in_slider_bar.current_value    = slider_bar->current_value;
        in_slider_bar.decimal_digits   = slider_bar->decimal_digits;
        in_slider_bar.step_value       = slider_bar->step_value;
        in_slider_bar.ruler_height     = slider_bar->ruler_height;
        in_slider_bar.slot_height      = slider_bar->slot_height;
        in_slider_bar.tick_width       = slider_bar->tick_width;
    }

    if ( (in_slider_bar.max_value) <= (in_slider_bar.min_value) )
        in_slider_bar.max_value = in_slider_bar.min_value + 100;
    if ( (in_slider_bar.current_value) < (in_slider_bar.min_value) )
        in_slider_bar.current_value = in_slider_bar.min_value;
    if ( (in_slider_bar.current_value) > (in_slider_bar.max_value) )
        in_slider_bar.current_value = in_slider_bar.max_value;
    if ( (in_slider_bar.step_value) < 1 )
        in_slider_bar.step_value = (in_slider_bar.max_value - in_slider_bar.min_value)/100;
    if ( (in_slider_bar.step_value) < 1 )
        in_slider_bar.step_value = 1;

    if ( (in_slider_bar.ruler_height) < SLBAR_RULER_HEIGHT )
        in_slider_bar.ruler_height = SLBAR_RULER_HEIGHT;
    if ( (in_slider_bar.slot_height) < SLBAR_SLOT_HEIGHT )
        in_slider_bar.slot_height = SLBAR_SLOT_HEIGHT;
    if ( (in_slider_bar.tick_width) < SLBAR_TICK_WIDTH )
        in_slider_bar.tick_width = SLBAR_TICK_WIDTH;


    /* ?? */
    rect = p->common.client_dc.rect;

    rect.left   += (in_slider_bar.tick_width)/2;
    rect.right  -= (in_slider_bar.tick_width)/2;
    rect.top    += in_slider_bar.ruler_height;
    rect.bottom -= 1;
    in_slider_bar.slider_rect  = rect;

    /* Calculate */
    rect = p->common.client_dc.rect;
    in_slider_bar.tick_rect  = rect;

    memcpy(p->ext, &in_slider_bar, sizeof(IN_GUI_SLIDER_BAR));

    /* 
     * add hwnd and post message.
     */
    ret = in_deal_add_hwnd(gui_common_widget, p, 0);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  slider_bar_create(HWND parent, void *gui_common_widget, void *gui_slider_bar)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_slider_bar_create(parent, gui_common_widget, gui_slider_bar);
    gui_unlock( );

    return  p;
}
#endif


int  in_slider_bar_get_value(HWND hwnd, int *value)
{
    HWND   p = hwnd;


    if ( value == NULL )
        return  -1;
    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != SLIDER_BAR_WIDGET_TYPE )
        return  -1;


    *value = GET_IN_GUI_SLIDER_BAR(p)->current_value;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  slider_bar_get_value(HWND hwnd, int *value)
{
    int  ret = 0;

    gui_lock( );
    ret = in_slider_bar_get_value(hwnd, value);
    gui_unlock( );

    return  ret;
}
#endif

int  in_slider_bar_get_percent(HWND hwnd, unsigned int *percent)
{
    HWND                 p      = hwnd;
    IN_GUI_SLIDER_BAR    in_slbar;
    UINT64               value = 0;
    int                  i     = 0;


    if ( percent == NULL )
        return  -1;
    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != SLIDER_BAR_WIDGET_TYPE )
        return  -1;


    in_slbar = *(GET_IN_GUI_SLIDER_BAR(p));
    value    = in_slbar.current_value - in_slbar.min_value;
    for( i = 0; i < (in_slbar.decimal_digits + 3) ; i++ )
         value *= 10;

    value  /= (in_slbar.max_value - in_slbar.min_value);
    value  += 5;

    *percent = (value/10);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  slider_bar_get_percent(HWND hwnd, unsigned int *percent)
{
    int  ret = 0;

    gui_lock( );
    ret = in_slider_bar_get_percent(hwnd, percent);
    gui_unlock( );

    return  ret;
}
#endif

int  in_slider_bar_set_value(HWND hwnd, int value)
{
    HWND   p = hwnd;


    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != SLIDER_BAR_WIDGET_TYPE )
        return  -1;



    if ( value < (GET_IN_GUI_SLIDER_BAR(p)->min_value) )
        return  -1;
    if ( value > (GET_IN_GUI_SLIDER_BAR(p)->max_value) )
        return  -1;

        
    GET_IN_GUI_SLIDER_BAR(p)->current_value = value;

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  slider_bar_set_value(HWND hwnd, int value)
{
    int  ret = 0;

    gui_lock( );
    ret = in_slider_bar_set_value(hwnd, value);
    gui_unlock( );

    return  ret;
}
#endif
     
int  in_slider_bar_set_percent(HWND hwnd, unsigned int percent)
{
    HWND    p      = hwnd;
    UINT64  offset = 0;
    UINT64  tmp    = 0;
    int     i      = 0;


    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != SLIDER_BAR_WIDGET_TYPE )
        return  -1;
            
    offset = percent*( GET_IN_GUI_SLIDER_BAR(p)->max_value - GET_IN_GUI_SLIDER_BAR(p)->min_value);
    for( i = 0; i < ((GET_IN_GUI_SLIDER_BAR(p)->decimal_digits) + 2) ; i++ )
        offset /= 10;

    tmp = GET_IN_GUI_SLIDER_BAR(p)->min_value + offset;
    if ( tmp < (GET_IN_GUI_SLIDER_BAR(p)->min_value) )
        tmp = GET_IN_GUI_SLIDER_BAR(p)->min_value;
    if ( tmp > (GET_IN_GUI_SLIDER_BAR(p)->max_value) ) 
        tmp = GET_IN_GUI_SLIDER_BAR(p)->max_value;


    GET_IN_GUI_SLIDER_BAR(p)->current_value = (int)tmp;

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  slider_bar_set_percent(HWND hwnd, unsigned int percent)
{
    int  ret = 0;

    gui_lock( );
    ret  = in_slider_bar_set_percent(hwnd, percent);
    gui_unlock( );

    return  ret;
}
#endif
    
#endif  /* _LG_SLIDER_BAR_WIDGET_ */
#endif  /* _LG_WINDOW_ */
