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

#include  <d2_circle.h>

#include  <keyboard.h>

#include  <win_tools.h>
#include  <win_callback.h>
#include  <win_invalidate.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_widget.h>
#include  <win_widget_group.h>
#include  <win_radio_button.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_RADIO_BUTTON_WIDGET_

/* RadioButton internal callback */
static  int  in_radio_button_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND                       p;
    HDC                        hdc;
    GUI_RECT                   rect;
    unsigned int               r;
    IN_GUI_RADIO_BUTTON       *pin_radio;

    unsigned int               cur_group;
    GUI_COLOR                  old_color;
    GUI_COLOR                  text_color;
    int                        key_value  = 0;
 

    if ( msg == NULL )
        return  -1;

    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;
        
    pin_radio = (IN_GUI_RADIO_BUTTON *)(p->ext);

    switch ( MESSAGE_GET_ID(msg) )
    {
        case  MSG_PAINT:
            if ( in_win_is_visual(p) < 1 )
                break;

            in_win_set_current_color_group(p);

            in_paint_widget_back(p);

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

            r = GUI_MIN(rect.right-rect.left+1, rect.bottom-rect.top+1);
            r = r/2 - 1;

            if ( (pin_radio->radius_offset) > (r - RBTN_MINI_RADIUS_OFFSET) )
                pin_radio->radius_offset = r - RBTN_MINI_RADIUS_OFFSET;
            if ( (pin_radio->radius_offset) < RBTN_MINI_RADIUS_OFFSET )
                pin_radio->radius_offset = RBTN_MINI_RADIUS_OFFSET;

            #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
            if ( (p->common.bimage_flag) > 0 )
                in_circle(hdc, (rect.right-rect.left)/2, (rect.bottom-rect.top)/2, r);
            else
            #endif
                in_circle_fill(hdc, (rect.right-rect.left)/2, (rect.bottom-rect.top)/2, r);

            if ((pin_radio->state) > 0)
            {
                cur_group  = in_hdc_get_current_group(hdc);
                old_color  = in_hdc_get_group_color(hdc, cur_group, FORE_ROLE );
                text_color = in_hdc_get_group_color(hdc, cur_group, TEXT_FORE_ROLE );
                in_hdc_set_group_color(hdc, cur_group, FORE_ROLE, text_color);
                in_circle_fill(hdc, (rect.right-rect.left)/2, (rect.bottom-rect.top)/2, r - pin_radio->radius_offset );
                in_hdc_set_group_color(hdc, cur_group, FORE_ROLE, old_color);
            }
            in_hdc_release_win(p, hdc);

            p->common.invalidate_flag = 0;

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_refresh( );
            #endif 

            in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_APP_CALLBACK);
            break;

        case  MSG_GET_FOCUS:
            in_radio_button_set_state(p, 1);
            break;

        case  MSG_KEY_DOWN:
            key_value = MESSAGE_GET_KEY_VALUE(msg);

            /* GUI_KEY_SPACE */            
            if ( key_value != GUI_KEY_SPACE )
                break;

            in_radio_button_set_state(p, 1);
            break;

        default:
            break;
    }

    return  in_common_widget_callback(msg);
}

HWND in_radio_button_create(HWND parent, void *gui_common_widget, void *gui_radio_button)
{
    HWND                  p = NULL;
    GUI_RADIO_BUTTON     *radio_button = (GUI_RADIO_BUTTON *)gui_radio_button;
    IN_GUI_RADIO_BUTTON   in_radio_button;
    unsigned int          size = 0;
    int                   ret;
   

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
    ret = in_set_hwnd_common1(parent, RADIO_BUTTON_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;


    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = RBTN_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = RBTN_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = RBTN_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = RBTN_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = RBTN_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = RBTN_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc                         = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = RBTN_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = RBTN_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = RBTN_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = RBTN_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = RBTN_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = RBTN_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = RBTN_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = RBTN_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = RBTN_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = RBTN_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = RBTN_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = RBTN_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc                    = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.in_callback = in_radio_button_callback;


    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_RADIO_BUTTON);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */

    memset(&in_radio_button, 0, sizeof(in_radio_button));

    in_radio_button.group_hwnd     = NULL;
    in_radio_button.state          = 0;
    in_radio_button.connected_flag = 0;

    if ( radio_button != NULL )
        in_radio_button.radius_offset = radio_button->radius_offset;
    else
        in_radio_button.radius_offset = RBTN_RADIUS_OFFSET;


    memcpy(p->ext, &in_radio_button, sizeof(IN_GUI_RADIO_BUTTON));

    /* 
     * add hwnd and post message.
     */
    ret = in_deal_add_hwnd(gui_common_widget, p, 0);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  radio_button_create(HWND parent, void *gui_common_widget, void *gui_radio_button)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_radio_button_create(parent, gui_common_widget, gui_radio_button);
    gui_unlock( );

    return  p;
}
#endif

int  in_radio_button_set_state(HWND hwnd, unsigned char state)
{
    HWND                   p          = hwnd;
    HWND                   pgroup     = hwnd;
    HWND                   tmp_p      = NULL;

    IN_GUI_WIDGET_GROUP   *pin_group  = NULL;
    IN_GUI_RADIO_BUTTON   *pin_radio1 = NULL;
    IN_GUI_RADIO_BUTTON   *pin_radio2 = NULL;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != RADIO_BUTTON_WIDGET_TYPE)
        return  -1;

    pin_radio1 = (IN_GUI_RADIO_BUTTON *)(p->ext);
    if ( pin_radio1 == NULL )
        return  -1;

 
    if ( (pin_radio1->state) == state )
        return  -1;


    if ( state == 0 )
        goto  RBTN_PAINT;

 
    pgroup = pin_radio1->group_hwnd;
    if ( (pgroup == NULL) )
        goto  RBTN_UNGROUP;
    
    if ((pin_radio1->connected_flag) == 0)
        goto  RBTN_UNGROUP;
 
    pin_group = (IN_GUI_WIDGET_GROUP *)(pgroup->ext);
    if ( pin_group == NULL )
        goto  RBTN_UNGROUP;


    if (pin_group->checked_hwnd == NULL)
        goto  RBTN_GROUP;
    if (pin_group->checked_flag == 0)
        goto  RBTN_GROUP;

    pin_radio2 = (IN_GUI_RADIO_BUTTON *)((pin_group->checked_hwnd)->ext);
    if ( pin_radio2 == NULL )
        goto  RBTN_GROUP;

    pin_radio2->state = 0;
    in_win_message_post_ext(pin_group->checked_hwnd, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK);


    RBTN_GROUP:
    pin_group->checked_hwnd = p;
    pin_group->checked_flag = 1;
    goto  RBTN_PAINT;


    RBTN_UNGROUP:
    if ((p->head.parent) == NULL)
        goto  RBTN_PAINT;
   
    tmp_p = (p->head.parent)->head.fc;
    for ( ; tmp_p != NULL; tmp_p = tmp_p->head.next )
    {
        if ((tmp_p->common.type) != RADIO_BUTTON_WIDGET_TYPE)
            continue;

        pin_radio2 = (IN_GUI_RADIO_BUTTON *)(tmp_p->ext);
        if ( pin_radio2 == NULL )
            continue;

        pgroup = pin_radio2->group_hwnd;
        if ( pgroup != NULL )
            continue;

        if ((pin_radio2->state) == 0)
            continue;

        pin_radio2->state = 0;
        in_win_message_post_ext(tmp_p, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK);
    }


    RBTN_PAINT:
    pin_radio1->state = state;
    in_win_message_post_ext(p, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  radio_button_set_state(HWND hwnd, unsigned char state)
{
    int  ret = 0;

    gui_lock( );
    ret = in_radio_button_set_state(hwnd, state);
    gui_unlock( );

    return  ret;
}
#endif

int  in_radio_button_get_state(HWND hwnd)
{
    HWND                   p = hwnd;
    IN_GUI_RADIO_BUTTON   *pin_radio;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != RADIO_BUTTON_WIDGET_TYPE)
        return  -1;

    pin_radio = (IN_GUI_RADIO_BUTTON *)(p->ext);
    if ( pin_radio == NULL )
        return  -1;

    return  pin_radio->state;
}

#ifndef  _LG_ALONE_VERSION_
int  radio_button_get_state(HWND hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_radio_button_get_state(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_RADIO_BUTTON_WIDGET_ */
#endif  /* _LG_WINDOW_ */
