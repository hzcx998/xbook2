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

#include  <keyboard.h>

#include  <win_tools.h>
#include  <win_callback.h>
#include  <win_invalidate.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_widget.h>
#include  <win_item_data.h>
#include  <win_list_box.h>
#include  <win_com_box.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_LIST_BOX_WIDGET_

/* ListBox internal callback */
static  int  in_list_box_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND             p;
    HDC              hdc;
    GUI_RECT         old_rect;
    GUI_RECT         rect;
    int              height = 0; 
    int              width  = 0;
    ITEM_DATA_LIST  *item_p = NULL;

    #ifdef   _LG_MTJT_
    GUI_SCBAR       *scbar  = NULL;
    #endif

    GUI_COLOR        old_color;
    int              tmp    = 0;

    int          key_value  = 0;

    #ifdef   _LG_MTJT_
    int              x      = 0;
    int              y      = 0;
    #endif

    int              flag   = 0;



    if ( msg == NULL )
        return  -1;

    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    hdc = in_hdc_get_client(p);
    if ( hdc == NULL )
        return  -1;

    rect = hdc->rect;     
    old_rect = rect;

    height = in_hdc_get_font_height(hdc);
    if ( height < MIN_LIST_BOX_ROW_HEIGHT )
        height = MIN_LIST_BOX_ROW_HEIGHT;

    width = in_hdc_get_font_width(hdc);
    if ( width < MIN_LIST_BOX_CHAR_WIDTH )
        width = MIN_LIST_BOX_CHAR_WIDTH;

    item_p = GET_IN_GUI_LIST_BOX(p)->pitem;

    switch ( MESSAGE_GET_ID(msg) )
    {
        case  MSG_PAINT:
            if ( in_win_is_visual(p) < 1 )
                break;

            in_win_set_current_color_group(p);

            /* Paint background */
            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            in_paint_widget_back(p);

            hdc = in_hdc_get_client(p);
            if ( hdc == NULL )
                return  -1;

            rect = hdc->rect;     
            old_rect = rect;

             /* paint border */
            if ( IS_BORDER_WIDGET(&(p->common)) > 0 )
            {
                if ( IS_BORDER_3D_WIDGET(&(p->common)) > 0 )
                    in_paint_3d_down_border(p, &(p->common.win_dc.rect));
                else
                    in_paint_widget_border(p);
            }


            /* paint item */
            rect      = GET_IN_GUI_LIST_BOX(p)->list_rect;
            hdc->rect = rect;

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
            #endif

            rect.right = GUI_RECTW(&rect) - 1; 
            rect.left  = (GET_IN_GUI_LIST_BOX(p)->x_step)*width;
            rect.top   = (GET_IN_GUI_LIST_BOX(p)->y_step)*height;
            tmp        = 0;

            while ( item_p != NULL )
            {
                rect.bottom = rect.top + height - 1; 

                if ( (GET_IN_GUI_LIST_BOX(p)->lighted_index) == tmp )
                {
                    old_color = in_hdc_get_back_color(hdc);
                    in_hdc_set_back_color(hdc, GUI_YELLOW);
                    in_rect_fill(hdc, rect.left, rect.top, rect.right, rect.bottom);
                    in_hdc_set_back_color(hdc, old_color);
                }

                flag = 0;
                if ( GET_IN_GUI_LIST_BOX(p)->multi_flag < 1 )
                {
                    if ( (GET_IN_GUI_LIST_BOX(p)->selected_index) == tmp )
                        flag = 1;
                } else {
                    if ( (item_p->selected) > 0 )
                    {
                        flag = 1;
                    }
                }
                if ( flag > 0 )
                {
                    old_color = in_hdc_get_back_color(hdc);
                    in_hdc_set_back_color(hdc, GUI_GREEN);
                    in_rect_fill(hdc, rect.left, rect.top, rect.right, rect.bottom);
                    in_hdc_set_back_color(hdc, old_color);
                }

                in_text_out_rect(hdc, &rect, item_p->pdata, -1, LG_TA_LEFT | LG_TA_TOP);

                rect.top  += height;
                item_p  = item_p->next;
                tmp++;
            }

            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_refresh( );
            #endif 

            /* Draw sbar */
            #ifdef  _LG_SCROLL_BAR_
            if (IS_HBAR_SCBAR(&(p->common)) > 0)
                in_paint_schbar(p);

            if (IS_VBAR_SCBAR(&(p->common)) > 0)
                in_paint_scvbar(p);
            #endif

            p->common.invalidate_flag = 0;

            in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_APP_CALLBACK);
            return  0;

        case  MSG_KEY_DOWN:
            if ( (GET_IN_GUI_LIST_BOX(p)->counter) < 1 )
                break  ;

            key_value = MESSAGE_GET_KEY_VALUE(msg);

            /* GUI_KEY_DOWN */            
            if ( (key_value != GUI_KEY_DOWN)&&(key_value != GUI_KEY_END) )
                goto  LBOX_KEY_DOWN_UP;

            tmp = GET_IN_GUI_LIST_BOX(p)->lighted_index;
            if ( key_value == GUI_KEY_DOWN)
                tmp++;
            else
                tmp += GET_IN_GUI_LIST_BOX(p)->y_multi_step;

            if ( tmp > ((GET_IN_GUI_LIST_BOX(p)->counter)-1) )
                tmp = 0;
            else if ( tmp < 0  )
                tmp = 0;
 
            GET_IN_GUI_LIST_BOX(p)->lighted_index = tmp;
            goto  LBOX_KEY_DOWN_END;

            /* GUI_KEY_UP */            
            LBOX_KEY_DOWN_UP:
            if ( (key_value != GUI_KEY_UP)&&(key_value != GUI_KEY_HOME) )
                goto  LBOX_KEY_DOWN_SPACE;

            tmp = GET_IN_GUI_LIST_BOX(p)->lighted_index;
            if ( key_value == GUI_KEY_UP)
                tmp--;
            else
                tmp -= GET_IN_GUI_LIST_BOX(p)->y_multi_step;

            if ( tmp < 0  )
                tmp = (GET_IN_GUI_LIST_BOX(p)->counter)-1;
            else if ( tmp > ((GET_IN_GUI_LIST_BOX(p)->counter)-1) )
                tmp = 0;

            GET_IN_GUI_LIST_BOX(p)->lighted_index = tmp;
            goto  LBOX_KEY_DOWN_END;


            /* GUI_KEY_SPACE */            
            LBOX_KEY_DOWN_SPACE:
            if ( key_value != GUI_KEY_SPACE )
                goto  LBOX_KEY_DOWN_RIGHT;

            tmp = GET_IN_GUI_LIST_BOX(p)->lighted_index;
            if ( tmp < 0 )
                goto  LBOX_KEY_DOWN_END;
            else if ( tmp > ((GET_IN_GUI_LIST_BOX(p)->counter)-1) )
                goto  LBOX_KEY_DOWN_END;

            /* Single mode */
            if ( (GET_IN_GUI_LIST_BOX(p)->multi_flag) > 0 )
                goto  LBOX_KEY_DOWN_MSPACE;

            if ( (GET_IN_GUI_LIST_BOX(p)->selected_index) != (GET_IN_GUI_LIST_BOX(p)->lighted_index) )
                GET_IN_GUI_LIST_BOX(p)->selected_index = GET_IN_GUI_LIST_BOX(p)->lighted_index;
            else
                GET_IN_GUI_LIST_BOX(p)->selected_index = -1;


            /* ?? */
            if ( (p->common.acc_hwnd_flag) > 0 )
            {
                in_com_box_set_index_item((p->head.parent), tmp);
                in_com_box_set_state(p->head.parent, 0);
          
                hdc->rect = old_rect;
                in_hdc_release_win(p, hdc);

                return  0;
            }

            goto  LBOX_KEY_DOWN_END;

            /* Multi mode */
            LBOX_KEY_DOWN_MSPACE:
            item_p = GET_IN_GUI_LIST_BOX(p)->pitem;
            tmp = 0;
            while( item_p != NULL )
            {
                if ( tmp != (GET_IN_GUI_LIST_BOX(p)->lighted_index) )
                    goto  LBOX_MSPACE_CONTINUE;

                if ( (item_p->selected) > 0 )
                    item_p->selected = 0;
                else
                    item_p->selected = 1;

                goto  LBOX_KEY_DOWN_END;

                LBOX_MSPACE_CONTINUE:
                item_p = item_p->next;
                tmp++;
            }
            break;

            /* GUI_KEY_RIGHT */
            LBOX_KEY_DOWN_RIGHT:
            if ( key_value != GUI_KEY_RIGHT )
                goto  LBOX_KEY_DOWN_LEFT;

            tmp = GUI_RECTW(&(GET_IN_GUI_LIST_BOX(p)->list_rect))/width;
            if ( (GET_IN_GUI_LIST_BOX(p)->max_text_len) < tmp )
                goto  LBOX_KEY_DOWN_END;

            GET_IN_GUI_LIST_BOX(p)->x_step -= 1;
            if ( (0-(GET_IN_GUI_LIST_BOX(p)->x_step)) > ((GET_IN_GUI_LIST_BOX(p)->max_text_len) - tmp))
                GET_IN_GUI_LIST_BOX(p)->x_step = tmp - (GET_IN_GUI_LIST_BOX(p)->max_text_len);

            goto  LBOX_KEY_DOWN_END;

            /* GUI_KEY_LEFT */
            LBOX_KEY_DOWN_LEFT:
            if ( key_value != GUI_KEY_LEFT )
                goto  LBOX_KEY_DOWN_DELETE;

            GET_IN_GUI_LIST_BOX(p)->x_step += 1;
            if ( (GET_IN_GUI_LIST_BOX(p)->x_step) > 0 )
                GET_IN_GUI_LIST_BOX(p)->x_step = 0;

            goto  LBOX_KEY_DOWN_END;

            /* GUI_KEY_DELETE */
            LBOX_KEY_DOWN_DELETE:
            if ( key_value != GUI_KEY_DELETE )
                break;

            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            in_list_box_delete_item(p, (GET_IN_GUI_LIST_BOX(p)->lighted_index));
            return  0;

            /* MSG_KEY_DOWN ok */
            LBOX_KEY_DOWN_END:
            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            in_list_box_update_lighted_index_rect(p);
            #ifdef  _LG_SCROLL_BAR_
            in_list_box_update_schbar_mid_rect(p);
            in_list_box_update_scvbar_mid_rect(p);
            #endif
            in_win_message_send_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            in_win_message_send_ext(p, MSG_NOTIFY_SEL_CHANGED, HWND_APP_CALLBACK);
            return  0;

        #ifdef   _LG_MTJT_
        case  MSG_MTJT_LBUTTON_UP:
            if ( item_p == NULL )
                goto  LBOX_CALLBACK_END;

            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_CALLBACK_END;

            rect  = GET_IN_GUI_LIST_BOX(p)->list_rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_HBAR;


            hdc->rect = rect;

            /* Clicked-item id (tmp) */
            tmp = (y-rect.top)/height - GET_IN_GUI_LIST_BOX(p)->y_step;
   
            /* Clicked item */
            if ( tmp <= ((GET_IN_GUI_LIST_BOX(p)->counter) - 1) )
            {
                GET_IN_GUI_LIST_BOX(p)->lighted_index  = tmp;
                goto  LBOX_LBUTTON_SELECT;
            }

            /* Clicked blank */
            if ( (GET_IN_GUI_LIST_BOX(p)->multi_flag) < 1 )
            {
                GET_IN_GUI_LIST_BOX(p)->selected_index = -1;
                GET_IN_GUI_LIST_BOX(p)->lighted_index  = -1;
                goto  LBOX_LBUTTON_END;
            }
            while ( item_p != NULL )
            {
                item_p->selected = 0;
                item_p = item_p->next;
            } 
            goto  LBOX_LBUTTON_END;

            LBOX_LBUTTON_SELECT:
            if ( GET_IN_GUI_LIST_BOX(p)->multi_flag < 1 )
            {
                GET_IN_GUI_LIST_BOX(p)->selected_index = tmp;
                GET_IN_GUI_LIST_BOX(p)->lighted_index  = tmp;
                /* ?? */
                if ( (p->common.acc_hwnd_flag) > 0 )
                {
                    in_com_box_set_index_item((p->head.parent), tmp);
                    in_com_box_set_state(p->head.parent, 0);
          
                    hdc->rect = old_rect;
                    in_hdc_release_win(p, hdc);

                    return  0;
                }

                goto  LBOX_LBUTTON_END;
            }

            flag = 0;
            while ( item_p != NULL )
            {
                if ( flag != tmp )
                {
                    flag++;
                    item_p = item_p->next;
                    continue;
                }

                if ( (item_p->selected) < 1 )
                {
                    item_p->selected = 1;
                    in_com_box_set_index_item((p->head.parent), tmp);
                    in_com_box_set_state(p->head.parent, 0);
                } else
                    item_p->selected = 0;

                break;
            }
            goto  LBOX_LBUTTON_END;

            /* HBAR */
            LBOX_LBUTTON_HBAR:
            scbar = in_get_schbar(p);
            if (scbar == NULL)
                goto  LBOX_LBUTTON_VBAR;

            /* The whole rect */
            rect = scbar->bar_rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_VBAR;

            /* fbtn_rect */
            rect = scbar->fbtn_rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_HBAR_LBTN;
        
            GET_IN_GUI_LIST_BOX(p)->x_step += 1;
            if ( (GET_IN_GUI_LIST_BOX(p)->x_step) > 0 )
                GET_IN_GUI_LIST_BOX(p)->x_step = 0;

            goto  LBOX_LBUTTON_END;

            /* lbtn_rect */
            LBOX_LBUTTON_HBAR_LBTN:
            rect = scbar->lbtn_rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_HBAR_BMID;

            tmp = GUI_RECTW(&(GET_IN_GUI_LIST_BOX(p)->list_rect))/width;
            if ( (GET_IN_GUI_LIST_BOX(p)->max_text_len) < tmp )
                goto  LBOX_LBUTTON_END;

            GET_IN_GUI_LIST_BOX(p)->x_step -= 1;
            if ( (0-(GET_IN_GUI_LIST_BOX(p)->x_step)) > ((GET_IN_GUI_LIST_BOX(p)->max_text_len) - tmp))
                GET_IN_GUI_LIST_BOX(p)->x_step = tmp - (GET_IN_GUI_LIST_BOX(p)->max_text_len);

            goto  LBOX_LBUTTON_END;

            /* Between fbtn_rect and mid_rect */
            LBOX_LBUTTON_HBAR_BMID:
            if ( (scbar->mid_rect.left) == ((scbar->fbtn_rect.right)+1) )
                goto  LBOX_LBUTTON_HBAR_NMID;

            rect = scbar->fbtn_rect;
            rect.left  = scbar->fbtn_rect.right + 1;
            rect.right = scbar->mid_rect.left - 1;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_HBAR_NMID;

            tmp = GUI_RECTW(&(GET_IN_GUI_LIST_BOX(p)->list_rect))/width;
            if ( (GET_IN_GUI_LIST_BOX(p)->max_text_len) < tmp )
                goto  LBOX_LBUTTON_END;

            GET_IN_GUI_LIST_BOX(p)->x_step += GET_IN_GUI_LIST_BOX(p)->x_multi_step;
            if ( (GET_IN_GUI_LIST_BOX(p)->x_step) > 0 )
                GET_IN_GUI_LIST_BOX(p)->x_step = 0;

            goto  LBOX_LBUTTON_END;

            /* Between mid_rect and lbtn_rect */
            LBOX_LBUTTON_HBAR_NMID:
            if ( (scbar->mid_rect.right) == ((scbar->lbtn_rect.left)-1) )
                goto  LBOX_LBUTTON_HBAR_MID;

            rect = scbar->fbtn_rect;
            rect.left  = scbar->mid_rect.right + 1;
            rect.right = scbar->lbtn_rect.left - 1;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_HBAR_MID;

            tmp = GUI_RECTW(&(GET_IN_GUI_LIST_BOX(p)->list_rect))/width;
            if ( (GET_IN_GUI_LIST_BOX(p)->max_text_len) < tmp )
                goto  LBOX_LBUTTON_END;

            GET_IN_GUI_LIST_BOX(p)->x_step -= GET_IN_GUI_LIST_BOX(p)->x_multi_step;
            if ( (0-(GET_IN_GUI_LIST_BOX(p)->x_step)) > ((GET_IN_GUI_LIST_BOX(p)->max_text_len) - tmp))
                GET_IN_GUI_LIST_BOX(p)->x_step = tmp - (GET_IN_GUI_LIST_BOX(p)->max_text_len);

            goto  LBOX_LBUTTON_END;

            /* mid_rect */
            LBOX_LBUTTON_HBAR_MID:
            goto  LBOX_LBUTTON_END;

            /* VBAR */
            LBOX_LBUTTON_VBAR:
            scbar = in_get_scvbar(p);
            if (scbar == NULL)
                goto  LBOX_LBUTTON_END;

            /* The whole rect */
            rect = scbar->bar_rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_END;

            /* fbtn_rect */
            rect = scbar->fbtn_rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_VBAR_LBTN;
        
            GET_IN_GUI_LIST_BOX(p)->y_step += 1;
            if ( (GET_IN_GUI_LIST_BOX(p)->y_step) > 0 )
                GET_IN_GUI_LIST_BOX(p)->y_step = 0;

            goto  LBOX_LBUTTON_END;

            /* lbtn_rect */
            LBOX_LBUTTON_VBAR_LBTN:
            rect = scbar->lbtn_rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_VBAR_BMID;

            tmp = GUI_RECTH(&(GET_IN_GUI_LIST_BOX(p)->list_rect))/height;
            if ( (GET_IN_GUI_LIST_BOX(p)->counter) < tmp )
                goto  LBOX_LBUTTON_END;

            GET_IN_GUI_LIST_BOX(p)->y_step -= 1;
            if ( (0-(GET_IN_GUI_LIST_BOX(p)->y_step)) > ((GET_IN_GUI_LIST_BOX(p)->counter) - tmp))
                GET_IN_GUI_LIST_BOX(p)->y_step = tmp - (GET_IN_GUI_LIST_BOX(p)->counter);

            goto  LBOX_LBUTTON_END;

            /* Between fbtn_rect and mid_rect */
            LBOX_LBUTTON_VBAR_BMID:
            if ( (scbar->mid_rect.top) == ((scbar->fbtn_rect.bottom)+1) )
                goto  LBOX_LBUTTON_VBAR_NMID;

            rect = scbar->fbtn_rect;
            rect.top    = scbar->fbtn_rect.bottom + 1;
            rect.bottom = scbar->mid_rect.top - 1;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_VBAR_NMID;

            tmp = GUI_RECTH(&(GET_IN_GUI_LIST_BOX(p)->list_rect))/height;
            if ( (GET_IN_GUI_LIST_BOX(p)->counter) < tmp )
                goto  LBOX_LBUTTON_END;

            GET_IN_GUI_LIST_BOX(p)->y_step += GET_IN_GUI_LIST_BOX(p)->y_multi_step;
            if ( (GET_IN_GUI_LIST_BOX(p)->y_step) > 0 )
                GET_IN_GUI_LIST_BOX(p)->y_step = 0;

            goto  LBOX_LBUTTON_END;

            /* Between mid_rect and lbtn_rect */
            LBOX_LBUTTON_VBAR_NMID:
            if ( (scbar->mid_rect.bottom) == ((scbar->lbtn_rect.top)-1) )
                goto  LBOX_LBUTTON_VBAR_MID;

            rect = scbar->fbtn_rect;
            rect.top    = scbar->mid_rect.bottom + 1;
            rect.bottom = scbar->lbtn_rect.top - 1;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  LBOX_LBUTTON_VBAR_MID;

            tmp = GUI_RECTH(&(GET_IN_GUI_LIST_BOX(p)->list_rect))/height;
            if ( (GET_IN_GUI_LIST_BOX(p)->counter) < tmp )
                goto  LBOX_LBUTTON_END;

            GET_IN_GUI_LIST_BOX(p)->y_step -= GET_IN_GUI_LIST_BOX(p)->y_multi_step;
            if ( (0-(GET_IN_GUI_LIST_BOX(p)->y_step)) > ((GET_IN_GUI_LIST_BOX(p)->counter) - tmp))
                GET_IN_GUI_LIST_BOX(p)->y_step = tmp - (GET_IN_GUI_LIST_BOX(p)->counter);

            goto  LBOX_LBUTTON_END;

            /* mid_rect */
            LBOX_LBUTTON_VBAR_MID:
            goto  LBOX_LBUTTON_END;

            LBOX_LBUTTON_END:
            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            #ifdef  _LG_SCROLL_BAR_
            in_list_box_update_schbar_mid_rect(p);
            in_list_box_update_scvbar_mid_rect(p);
            #endif

            in_win_message_send_ext(p, MSG_PAINT, HWND_IN_CALLBACK);
            in_win_message_send_ext(p, MSG_NOTIFY_SEL_CHANGED, HWND_APP_CALLBACK);
            return  0;
        #endif

        default:
            break;
    }
        
    #ifdef   _LG_MTJT_
    LBOX_CALLBACK_END:
    #endif        
    hdc->rect = old_rect;
    in_hdc_release_win(p, hdc);

    return  in_common_widget_callback(msg);
}

/* ListBox internal delete callback */
static  int  in_list_box_delete_callback(/* HWND */ void *hwnd)
{
    HWND             p = hwnd;
    ITEM_DATA_LIST  *item1_p;
    ITEM_DATA_LIST  *item2_p;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;

    item1_p = GET_IN_GUI_LIST_BOX(p)->pitem;
    while( item1_p != NULL )
    {
        item2_p = item1_p->next;
        free((void *)item1_p);
        item1_p = item2_p;
    }

    GET_IN_GUI_LIST_BOX(p)->pitem          = NULL;
    GET_IN_GUI_LIST_BOX(p)->counter        = 0;
    GET_IN_GUI_LIST_BOX(p)->selected_index = -1;
    GET_IN_GUI_LIST_BOX(p)->lighted_index  = -1;
    return  1;
}

HWND in_list_box_create(HWND parent, void *gui_common_widget, void *gui_list_box)
{
    HWND                p  = NULL;
    GUI_LIST_BOX       *list_box = (GUI_LIST_BOX *)gui_list_box;
    IN_GUI_LIST_BOX     in_list_box;

    GUI_RECT            rect;
    unsigned int        size = 0;
    int                 ret;
   

    /* ?? acc_hwnd_flag */
    if ( gui_common_widget == NULL )
        return  NULL;


    /* Adjust parent */
    if ( parent == NULL )
        parent = HWND_DESKTOP;

    if ( ((GUI_COMMON_WIDGET *)gui_common_widget)->acc_hwnd_flag < 1 )
    {
        if ( in_win_has(parent) < 0 )
            return  NULL;
    }


    /* 
     * set hwnd common 
     */

    /* set hwnd common1 */
    /* ID, style, ext_style, default win_dc */
    /* Border ?? */
    /* set ltcomm and ltdc value */
    ret = in_set_hwnd_common1(parent, LIST_BOX_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;


    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = LBOX_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = LBOX_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = LBOX_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = LBOX_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = LBOX_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = LBOX_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc                         = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = LBOX_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = LBOX_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = LBOX_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = LBOX_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = LBOX_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = LBOX_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = LBOX_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = LBOX_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = LBOX_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = LBOX_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = LBOX_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = LBOX_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc                    = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.in_callback         = in_list_box_callback;
    ltcomm.is_delete_callback  = 1;
    ltcomm.delete_callback     = in_list_box_delete_callback; 


    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_LIST_BOX);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */

    memset(&in_list_box, 0, sizeof(in_list_box));

    rect = ltcomm.client_dc.rect; 

    #ifdef  _LG_SCROLL_BAR_
    if (IS_HBAR_SCBAR(&ltcomm) > 0 )
        rect.bottom            -= lsbahw + 1;

    if (IS_VBAR_SCBAR(&ltcomm) > 0 )
        rect.right             -= lsbahw + 1;
    #endif 
    in_list_box.list_rect       = rect;

    in_list_box.pitem           = NULL; 
    in_list_box.counter         = 0;
    in_list_box.max_text_len    = 1;
    if ( list_box != NULL )
        in_list_box.multi_flag  = list_box->multi_flag;
    else
        in_list_box.multi_flag  = 0;
    in_list_box.selected_index  = -1;
    in_list_box.lighted_index   = -1;
    in_list_box.x_step          = 0;
    in_list_box.y_step          = 0;
    in_list_box.x_multi_step    = SCBAR_X_MULTI_STEP;
    in_list_box.y_multi_step    = SCBAR_Y_MULTI_STEP;
    in_list_box.read_only       = 0;

    memcpy(p->ext, &in_list_box, sizeof(IN_GUI_LIST_BOX));


    /* Scroll hbar and vbar */
    #ifdef  _LG_SCROLL_BAR_
    if ( lsbahw < SCBAR_HEIGHT_WIDTH )
        lsbahw = SCBAR_HEIGHT_WIDTH;
    #endif

    #ifdef  _LG_SCROLL_BAR_
    if (IS_HBAR_SCBAR(&ltcomm) < 1)
        goto  HBAR_OK;

    memset((void *)(&lscbar), 0, sizeof(GUI_SCBAR));

    rect                         = ltcomm.client_dc.rect;

    rect.top                     = rect.bottom-lsbahw+1;
    if (IS_VBAR_SCBAR(&ltcomm) > 0)
        rect.right -= lsbahw + 1;

    lscbar.bar_rect              = rect;

    rect.left                   += 3;
    rect.right                   = rect.left+lsbahw-1-6;
    rect.top                    += 3;
    rect.bottom                 -= 3;
    lscbar.fbtn_rect             = rect;

    lscbar.lbtn_rect             = lscbar.fbtn_rect;
    lscbar.lbtn_rect.right       = lscbar.bar_rect.right-3; 
    lscbar.lbtn_rect.left        = lscbar.lbtn_rect.right-lsbahw+1+6;

    lscbar.mid_rect              = lscbar.fbtn_rect;
    lscbar.mid_rect.left         = lscbar.fbtn_rect.right+1; 
    lscbar.mid_rect.right        = lscbar.lbtn_rect.left-1;

    lscbar.mid_len               = lscbar.mid_rect.right - lscbar.mid_rect.left + 1;

    memcpy((void *)(p->common.schbar), (const void *)(&lscbar), sizeof(GUI_SCBAR));

    HBAR_OK:
    if (IS_VBAR_SCBAR(&ltcomm) < 1)
        goto  VBAR_OK;

    memset((void *)(&lscbar), 0, sizeof(GUI_SCBAR));

    rect                         = ltcomm.client_dc.rect;

    rect.left                    = rect.right-lsbahw+1;
    if (IS_HBAR_SCBAR(&ltcomm) > 0)
        rect.bottom -= lsbahw + 1;
   
    lscbar.bar_rect              = rect;

    rect.left                   += 3;
    rect.right                  -= 3;
    rect.top                    += 3;
    rect.bottom                  = rect.top+lsbahw-1-6;
    lscbar.fbtn_rect             = rect;

    lscbar.lbtn_rect             = lscbar.fbtn_rect;
    lscbar.lbtn_rect.bottom      = lscbar.bar_rect.bottom-3; 
    lscbar.lbtn_rect.top         = lscbar.lbtn_rect.bottom-lsbahw+1+6;

    lscbar.mid_len               = lscbar.lbtn_rect.top - lscbar.fbtn_rect.bottom - 1;

    lscbar.mid_rect              = lscbar.fbtn_rect;
    lscbar.mid_rect.top          = lscbar.fbtn_rect.bottom+1; 
    lscbar.mid_rect.bottom       = lscbar.lbtn_rect.top-1;

    memcpy((void *)(p->common.scvbar), (const void *)(&lscbar), sizeof(GUI_SCBAR));
    VBAR_OK:
    #endif


    /* 
     * add hwnd and post message.
     */
    ret = in_deal_add_hwnd(gui_common_widget, p, ((GUI_COMMON_WIDGET *)gui_common_widget)->acc_hwnd_flag);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  list_box_create(HWND parent, void *gui_common_widget, void *gui_list_box)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_list_box_create(parent, gui_common_widget, gui_list_box);
    gui_unlock( );

    return  p;
}
#endif

int  in_list_box_update_lighted_index_rect(/* HWND hwnd */ void *hwnd)
{
    HWND   p      = hwnd;
    HDC    hdc;
    int    height = 0; 
    int    tmp1   = 0;
    int    tmp2   = 0;
    int    tmp3   = 0;

    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;


    tmp1 = GET_IN_GUI_LIST_BOX(p)->lighted_index; 
    tmp2 = 0 - GET_IN_GUI_LIST_BOX(p)->y_step;

    /* Above list_rect case */        
    if ( tmp1 < tmp2 )
    {
        tmp2 = tmp1 - 1;
        goto  LBOX_LIGHTED_INDEX_END;
    }


    /* Inside list_rect case */


    /* Below list_rect case */
    hdc    = in_hdc_get_client(p);
    height = in_hdc_get_font_height(hdc);
    if ( height < 4 )  
        height = 16;        
    in_hdc_release_win(p, hdc);

    tmp3 = (GUI_RECTH(&(GET_IN_GUI_LIST_BOX(p)->list_rect))/height);
    if  (tmp1 > (tmp2 + tmp3 - 1))
        tmp2 = tmp1 - tmp3 + 1;

    /* Set y_step value */
    LBOX_LIGHTED_INDEX_END:
    GET_IN_GUI_LIST_BOX(p)->y_step = 0 - tmp2;

    return  1;
}
    
#ifdef  _LG_SCROLL_BAR_
int  in_list_box_update_schbar_mid_rect(/* HWND hwnd */ void *hwnd)
{
    HWND         p      = hwnd;
    GUI_SCBAR   *scbar  = NULL;
    HDC          hdc;
    int          width  = 0; 
    int          tmp    = 0;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;

    if ( IS_HBAR_SCBAR(&(p->common)) < 1 )
        return  -1;


    scbar = in_get_schbar(p);
    if (scbar == NULL)
        return  -1;

    hdc   = in_hdc_get_client(p);
    width = in_hdc_get_font_width(hdc);
    if ( width < MIN_LIST_BOX_CHAR_WIDTH )
        width = MIN_LIST_BOX_CHAR_WIDTH;
    in_hdc_release_win(p, hdc);

    tmp = (GET_IN_GUI_LIST_BOX(p)->max_text_len)*width;
    if ( tmp <= GUI_RECTW(&((GET_IN_GUI_LIST_BOX(p)->list_rect))) )
        return  0;

    if ( (GET_IN_GUI_LIST_BOX(p)->x_step) > 0 )
        GET_IN_GUI_LIST_BOX(p)->x_step = 0;

    tmp = -((GET_IN_GUI_LIST_BOX(p)->x_step)*(scbar->mid_len))/(GET_IN_GUI_LIST_BOX(p)->max_text_len);
    if ( tmp < 1 )
        tmp = 1;
    if ( tmp > scbar->mid_len )
        tmp = scbar->mid_len;
    scbar->mid_rect.left = scbar->fbtn_rect.right + tmp;

    tmp  =(scbar->mid_len)*GUI_RECTW(&(GET_IN_GUI_LIST_BOX(p)->list_rect));
    tmp /= (GET_IN_GUI_LIST_BOX(p)->max_text_len)*width;
 
    scbar->mid_rect.right = scbar->mid_rect.left + tmp;

    if ( (scbar->mid_rect.left) < ((scbar->fbtn_rect.right)+1) )
        scbar->mid_rect.left =  scbar->fbtn_rect.right + 1;
    if ( (scbar->mid_rect.right) > ((scbar->lbtn_rect.left)-1) )
        scbar->mid_rect.right =  scbar->lbtn_rect.left - 1;

    if ( (scbar->mid_rect.right) < (scbar->mid_rect.left) )
        scbar->mid_rect.right = scbar->mid_rect.left;

    return  1;
}
    
int  in_list_box_update_scvbar_mid_rect(/* HWND hwnd */ void *hwnd)
{
    HWND         p       = hwnd;
    GUI_SCBAR   *scbar   = NULL;
    HDC          hdc;
    int          height  = 0; 
    int          tmp1    = 0;
    int          tmp2    = 0;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;

    if ( IS_VBAR_SCBAR(&(p->common)) < 1 )
        return  -1;


    scbar = in_get_scvbar(p);
    if (scbar == NULL)
        return  -1;

    hdc    = in_hdc_get_client(p);
    height = in_hdc_get_font_height(hdc);
    if ( height < MIN_LIST_BOX_ROW_HEIGHT )
        height = MIN_LIST_BOX_ROW_HEIGHT;
    in_hdc_release_win(p, hdc);

    tmp1 = (GET_IN_GUI_LIST_BOX(p)->counter)*height;
    if ( tmp1 <= GUI_RECTH(&((GET_IN_GUI_LIST_BOX(p)->list_rect))) )
        return  0;

    if ( (GET_IN_GUI_LIST_BOX(p)->y_step) > 0 )
        GET_IN_GUI_LIST_BOX(p)->y_step = 0;

    tmp1 = -((GET_IN_GUI_LIST_BOX(p)->y_step)*(scbar->mid_len))/(GET_IN_GUI_LIST_BOX(p)->counter);
    if ( tmp1 < 1 )
        tmp1 = 1;
    if ( tmp1 > scbar->mid_len )
        tmp1 = scbar->mid_len;
    scbar->mid_rect.top = scbar->fbtn_rect.bottom + tmp1;

    tmp1 = ((GET_IN_GUI_LIST_BOX(p)->counter)+(GET_IN_GUI_LIST_BOX(p)->y_step))*height - GUI_RECTH(&(GET_IN_GUI_LIST_BOX(p)->list_rect));
    if ( tmp1 < 0 )
    {
        scbar->mid_rect.bottom = scbar->lbtn_rect.top - 1;
        return  1;
    }

    tmp2 = (tmp1*(scbar->mid_len))/((GET_IN_GUI_LIST_BOX(p)->counter)*height);
    scbar->mid_rect.bottom = scbar->lbtn_rect.top - tmp2;

    if ( (scbar->mid_rect.top) < ((scbar->fbtn_rect.bottom)+1) )
        scbar->mid_rect.top =  scbar->fbtn_rect.bottom + 1;
    if ( (scbar->mid_rect.bottom) > ((scbar->lbtn_rect.top)-1) )
        scbar->mid_rect.bottom =  scbar->lbtn_rect.top - 1;

    if ( (scbar->mid_rect.bottom) < (scbar->mid_rect.top) )
        scbar->mid_rect.bottom = scbar->mid_rect.top;

    return  1;
}    
#endif

int  in_list_box_insert_item(HWND hwnd, int index, TCHAR  *text, unsigned int  text_len)
{
    HWND             p = hwnd;
    ITEM_DATA_LIST  *item1_p;
    ITEM_DATA_LIST  *item2_p;
    unsigned int     id = 0;


    if ( p == NULL )
        return  -1;
    if ( text == NULL )
        return  -1;
    if ( text_len < 1 )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;

    /* LBOX_INSERT_ITEM: */
    item2_p = malloc(sizeof(ITEM_DATA_LIST)+sizeof(TCHAR)*text_len+1); 
    if ( item2_p == NULL )
        return  -1;
    memset(item2_p, 0, sizeof(ITEM_DATA_LIST)+sizeof(TCHAR)*text_len+1);

    item2_p->prev     = NULL;
    item2_p->next     = NULL;
    item2_p->selected = 0;
    item2_p->len      = text_len;
    /* ?? */
    /* item2_p->pdata    = (TCHAR *)((void *)item2_p+sizeof(ITEM_DATA_LIST)); */
    item2_p->pdata    = (TCHAR *)((char *)item2_p+sizeof(ITEM_DATA_LIST));

    memcpy(item2_p->pdata, text, sizeof(TCHAR)*text_len);


    item1_p = GET_IN_GUI_LIST_BOX(p)->pitem;

    /* The first item */
    if ( item1_p == NULL )
    {
        GET_IN_GUI_LIST_BOX(p)->pitem = item2_p;
        goto  LBOX_INSERT_ITEM_END;
    }

    /* The 0 index */
    if ( index == 0 )
    {
        item2_p->next = item1_p;
        item1_p->prev = item2_p;
        GET_IN_GUI_LIST_BOX(p)->pitem = item2_p;
        goto  LBOX_INSERT_ITEM_END;
    }

    /* The other index */
    id = 0;
    while( (item1_p->next) != NULL )
    {
        if ( index != id )
            goto  LBOX_INSERT_ITEM_CONTINUE;


        LBOX_INSERT_ITEM_FIND:
        item2_p->prev = item1_p->prev;
        item2_p->next = item1_p;

        (item1_p->prev)->next = item2_p;

        item1_p->prev = item2_p;
        goto  LBOX_INSERT_ITEM_END;

        LBOX_INSERT_ITEM_CONTINUE:
        item1_p = item1_p->next;
        id++;
    }

    if ( index == id )
        goto  LBOX_INSERT_ITEM_FIND;

    item2_p->prev = item1_p;
    item1_p->next = item2_p;

    LBOX_INSERT_ITEM_END:

    GET_IN_GUI_LIST_BOX(p)->counter += 1;
    if ( (GET_IN_GUI_LIST_BOX(p)->max_text_len) < text_len )
        GET_IN_GUI_LIST_BOX(p)->max_text_len = text_len;

    #ifdef  _LG_SCROLL_BAR_
    in_list_box_update_schbar_mid_rect(p);
    in_list_box_update_scvbar_mid_rect(p);
    #endif

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  list_box_insert_item(HWND hwnd, int index, TCHAR  *text, unsigned int  text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_insert_item(hwnd, index, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif

int   in_list_box_get_item(HWND hwnd, unsigned int index, TCHAR *text, unsigned int *text_len)
{
    HWND             p        = hwnd;
    ITEM_DATA_LIST  *item_p   = NULL;
    unsigned int     id       = 0;
    unsigned int     copy_len = 0;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;


    if ( text == NULL )
        return  -1;
    if ( text_len == NULL )
        return  -1;
    if ( *text_len < 1 )
        return  -1;


    if ( (GET_IN_GUI_LIST_BOX(p)->counter) < 1 )
        return  0;
    if ( index > ((GET_IN_GUI_LIST_BOX(p)->counter) - 1) )
        return  -1;

    id = 0;
    item_p = GET_IN_GUI_LIST_BOX(p)->pitem;
    while ( item_p != NULL )
    {
        if ( index != id )
            goto  LBOX_GET_CONTINUE;
    
        copy_len = (item_p->len) < *text_len ? (item_p->len) : *text_len;
        memcpy(text, item_p->pdata, copy_len);
        *text_len = copy_len;
        return  1;

        LBOX_GET_CONTINUE:
        item_p = item_p->next;
        id++;
    }

    return  0;
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_get_item(HWND hwnd, unsigned int index, TCHAR *text, unsigned int *text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_get_item(hwnd, index, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif
 
int  in_list_box_delete_item(HWND hwnd, unsigned int index)
{
    HWND             p = hwnd;
    ITEM_DATA_LIST  *item1_p;
    ITEM_DATA_LIST  *item2_p;
    unsigned int     id = 0;
    unsigned int     text_len = 0;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;


    if ( (GET_IN_GUI_LIST_BOX(p)->counter) < 1 )
        return  0;
    if ( index > ((GET_IN_GUI_LIST_BOX(p)->counter) - 1) )
        return  0;

    if ( GET_IN_GUI_LIST_BOX(p)->read_only > 0 )
        return  -1;


    /* Looking for the item */
    item1_p = GET_IN_GUI_LIST_BOX(p)->pitem;
    id      = 0;
    while( item1_p != NULL )
    {
        if ( id == index )
            goto  LBOX_DELETE_ITEM_UPDATE;

        item1_p = item1_p->next;
        id++;
    }
    return  -1;

    LBOX_DELETE_ITEM_UPDATE:
    /* update prev */
    item2_p = item1_p->prev;
    if ( (item2_p) != NULL )
        item2_p->next = item1_p->next;

    /* update next */
    item2_p = item1_p->next;
    if ( (item2_p) != NULL )
        item2_p->prev = item1_p->prev;

    /* update GET_IN_GUI_LIST_BOX(p)->pitem */
    if ( id == 0 )
        GET_IN_GUI_LIST_BOX(p)->pitem = item2_p;
 
    /* update the GET_IN_GUI_LIST_BOX(p) other var */
    GET_IN_GUI_LIST_BOX(p)->counter        -= 1;
    GET_IN_GUI_LIST_BOX(p)->selected_index  = -1;
    text_len = item1_p->len;

    free((void *)item1_p);
    item1_p = NULL;

    /* update max_text_len */
    if ( text_len < (GET_IN_GUI_LIST_BOX(p)->max_text_len) )
        goto  LBOX_DELETE_ITEM_END;

    item1_p  = GET_IN_GUI_LIST_BOX(p)->pitem;
    text_len = 1;
    while( item1_p != NULL )
    {
        if ( (item1_p->len) > text_len )
            text_len = item1_p->len;

        item1_p = item1_p->next;

    } 
    (GET_IN_GUI_LIST_BOX(p)->max_text_len) = text_len;

    LBOX_DELETE_ITEM_END:
    #ifdef  _LG_SCROLL_BAR_
    in_list_box_update_schbar_mid_rect(p);
    in_list_box_update_scvbar_mid_rect(p);
    #endif

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  list_box_delete_item(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_delete_item(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif
    
int   in_list_box_set_select_mode(HWND hwnd, unsigned int  mode)
{
    HWND             p = hwnd;
    ITEM_DATA_LIST  *item_p = NULL;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;

    if ( ((GET_IN_GUI_LIST_BOX(p)->multi_flag) > 0) && (mode > 0) )
        return  1;
    if ( ((GET_IN_GUI_LIST_BOX(p)->multi_flag) < 1) && (mode < 1) )
        return  1;

     /* Set mulit mode */
    if ( (GET_IN_GUI_LIST_BOX(p)->multi_flag) < 1 )
    {
        GET_IN_GUI_LIST_BOX(p)->multi_flag     = 1;
        GET_IN_GUI_LIST_BOX(p)->selected_index = -1;
        return  1;
    }

    /* Set single mode */
    GET_IN_GUI_LIST_BOX(p)->multi_flag     = 0;
    GET_IN_GUI_LIST_BOX(p)->selected_index = -1;
 
    item_p = GET_IN_GUI_LIST_BOX(p)->pitem;
    while( item_p != NULL )
    {
        item_p->selected = 0;
        item_p = item_p->next;
    }

    #ifdef  _LG_SCROLL_BAR_
    in_list_box_update_schbar_mid_rect(p);
    in_list_box_update_scvbar_mid_rect(p);
    #endif

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
 
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  list_box_set_select_mode(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_set_select_mode(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif
    
int   in_list_box_get_select_mode(HWND hwnd, unsigned int *mode)
{
    HWND  p = hwnd;


    if ( mode == NULL )
        return  -1;

    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;

    *mode = GET_IN_GUI_LIST_BOX(p)->multi_flag;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_get_select_mode(HWND hwnd, unsigned int *mode)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_get_select_mode(hwnd, mode);
    gui_unlock( );

    return  ret;
}
#endif
 
int   in_list_box_set_selected_index(HWND hwnd, unsigned int index)
{
    HWND             p      = hwnd;
    ITEM_DATA_LIST  *item_p = NULL;
    unsigned  int    id     = 0;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;


    if ( (GET_IN_GUI_LIST_BOX(p)->counter) < 1 )
        return  0;
    if ( index > ((GET_IN_GUI_LIST_BOX(p)->counter) - 1) )
        return  -1;


    if ( (GET_IN_GUI_LIST_BOX(p)->multi_flag) >  0 )
        goto  LBOX_SET_MULTI_SELECT;

    GET_IN_GUI_LIST_BOX(p)->selected_index = index;
    goto  LBOX_SET_END;


    LBOX_SET_MULTI_SELECT:
    id = 0;
    item_p = GET_IN_GUI_LIST_BOX(p)->pitem;
    while( item_p != NULL )
    {
        if ( index != id )
            goto  LBOX_SET_MULTI_NEXT;

        item_p->selected = 1;
        break;

        LBOX_SET_MULTI_NEXT:
        item_p = item_p->next;
        id++;
    }

    LBOX_SET_END:
    #ifdef  _LG_SCROLL_BAR_
    in_list_box_update_schbar_mid_rect(p);
    in_list_box_update_scvbar_mid_rect(p);
    #endif

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
 
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_set_selected_index(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_set_selected_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif
  
int   in_list_box_is_selected_index(HWND hwnd, unsigned int index)
{
    HWND             p      = hwnd;
    ITEM_DATA_LIST  *item_p = NULL;
    unsigned int     id     = 0;


    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != LIST_BOX_WIDGET_TYPE )
        return  -1;
 

    if ( (GET_IN_GUI_LIST_BOX(p)->multi_flag)  > 0 )
        goto  LBOX_MULTI_SELECT;

    /* Single mode */
    if ( (GET_IN_GUI_LIST_BOX(p)->selected_index) == index )
        return  1;

    return  0;

    /* Multi mode */
    LBOX_MULTI_SELECT:
    id = 0;
    item_p = GET_IN_GUI_LIST_BOX(p)->pitem;
    while ( item_p != NULL )
    {
        if ( (item_p->selected) <  1 )    
            goto  LBOX_MULTI_CONTINUE;
    
        if ( index == id )
            return  1;

        LBOX_MULTI_CONTINUE:
        item_p = item_p->next;
        id++;
    }

    return  0;
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_is_selected_index(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_is_selected_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif

int   in_list_box_get_selected_index(HWND hwnd, unsigned int start_index, int *index)
{
    HWND             p      = hwnd;
    ITEM_DATA_LIST  *item_p = NULL;
    unsigned int     id     = 0;


    if  ( index == NULL )
        return  -1;

    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != LIST_BOX_WIDGET_TYPE )
        return  -1;


    *index = -1;

    if ( (GET_IN_GUI_LIST_BOX(p)->multi_flag)  > 0 )
        goto  LBOX_MULTI_SELECT;

    /* Single mode */
    if ( (GET_IN_GUI_LIST_BOX(p)->selected_index) < start_index )
        return  0;

    *index = GET_IN_GUI_LIST_BOX(p)->selected_index;
    return  1;

    /* Multi mode */
    LBOX_MULTI_SELECT:
    id = 0;
    item_p = GET_IN_GUI_LIST_BOX(p)->pitem;
    while ( item_p != NULL )
    {
        if ( (item_p->selected) <  1 )    
            goto  LBOX_MULTI_CONTINUE;

        if ( start_index > id )
            goto  LBOX_MULTI_CONTINUE;

        *index = id;
        return  1;

        LBOX_MULTI_CONTINUE:
        item_p = item_p->next;
        id++;
    }

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_get_selected_index(HWND hwnd, unsigned int start_index, int *index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_get_selected_index(hwnd, start_index, index);
    gui_unlock( );

    return  ret;
}
#endif

int   in_list_box_set_lighted_index(HWND hwnd, unsigned int index)
{
    HWND  p = hwnd;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;


    if ( (index + 1) > (GET_IN_GUI_LIST_BOX(p)->counter)  )
        return  -1;

    (GET_IN_GUI_LIST_BOX(p)->lighted_index) = index;

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_set_lighted_index(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_set_lighted_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif
     
int   in_list_box_is_lighted_index(HWND hwnd, unsigned int index)
{
    HWND  p = hwnd;

    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;


    return  ((GET_IN_GUI_LIST_BOX(p)->lighted_index) == index);
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_is_lighted_index(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_is_lighted_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif

int   in_list_box_get_lighted_index(HWND hwnd, int *index)
{
    HWND  p = hwnd;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;


    if ( index == NULL )
        return  -1;

    *index = GET_IN_GUI_LIST_BOX(p)->lighted_index;
    return  *index;
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_get_lighted_index(HWND hwnd, int *index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_get_lighted_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif
 
int   in_list_box_get_item_counter(HWND hwnd, unsigned int *counter)
{
    HWND  p = hwnd;

    if ( p == NULL )
        return  -1;
    if ((p->common.type) != LIST_BOX_WIDGET_TYPE)
        return  -1;

    if ( counter == NULL )
        return  -1;

    *counter = GET_IN_GUI_LIST_BOX(p)->counter;
    return  *counter;
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_get_item_counter(HWND hwnd, unsigned int *counter)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_get_item_counter(hwnd, counter);
    gui_unlock( );

    return  ret;
}
#endif
    
int   in_list_box_is_read_only(HWND hwnd)
{
    HWND   p = hwnd;

    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != LIST_BOX_WIDGET_TYPE )
        return  -1;

    return  (GET_IN_GUI_LIST_BOX(p)->read_only);
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_is_read_only(HWND hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_is_read_only(hwnd);
    gui_unlock( );

    return  ret;
}
#endif
   
int   in_list_box_set_read_only(HWND hwnd, unsigned int read_only)
{
    HWND   p = hwnd;

    if ( p == NULL )
        return  -1;
    if ( (p->common.type) != LIST_BOX_WIDGET_TYPE )
        return  -1;

    if ( read_only > 0 )
        GET_IN_GUI_LIST_BOX(p)->read_only = 1;
    else
        GET_IN_GUI_LIST_BOX(p)->read_only = 0;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int   list_box_set_read_only(HWND hwnd, unsigned int read_only)
{
    int  ret = 0;

    gui_lock( );
    ret = in_list_box_set_read_only(hwnd, read_only);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_LIST_BOX_WIDGET_ */
#endif  /* _LG_WINDOW_ */
