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

#include  <d2_rect.h>
#include  <d2_triangle.h>

#include  <win_tools.h>
#include  <win_callback.h>
#include  <win_invalidate.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_widget.h>
#include  <win_line_edit.h> 
#include  <win_list_box.h> 
#include  <win_com_box.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_COM_BOX_WIDGET_

static TCHAR  glebuf[MAX_LINE_EDIT_TEXT_LEN+1] = "";


/* ComBox internal callback */
static  int  in_com_box_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND         p = NULL;
    HDC          hdc;
    GUI_RECT     rect;
    GUI_RECT     old_rect;
    GUI_COLOR    old_color;

    HWND         hline_edit = NULL;
    HWND         hlist_box  = NULL;

    int          key_value = 0;

    #ifdef  _LG_MTJT_
    int          x     = 0;
    int          y     = 0;
    #endif


 
    if ( msg == NULL )
        return  -1;

    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    if ( (GET_IN_GUI_COM_BOX(p)->state) > 0 )
    { 
        p->common.win_dc.rect     = GET_IN_GUI_COM_BOX(p)->opened_win_rect; 
        p->common.client_dc.rect  = GET_IN_GUI_COM_BOX(p)->opened_cli_rect;
    } else {
        p->common.win_dc.rect     = GET_IN_GUI_COM_BOX(p)->closed_win_rect; 
        p->common.client_dc.rect  = GET_IN_GUI_COM_BOX(p)->closed_cli_rect; 
    }

    hline_edit = GET_IN_GUI_COM_BOX(p)->pline_edit;
    if ( hline_edit == NULL )
        return  -1; 

    hlist_box = GET_IN_GUI_COM_BOX(p)->plist_box;
    if ( hlist_box == NULL )
        return  -1;

    switch ( MESSAGE_GET_ID(msg) )
    {
        case  MSG_PAINT:
            if ( in_win_is_visual(p) < 1 )
                break;

            hdc = in_hdc_get_client(p);
            if ( hdc == NULL )
                break;

            rect = hdc->rect;     
            old_rect = rect;

            /* Set current color group */
            in_win_set_current_color_group(p);

            hline_edit->common.win_dc.cur_group    = p->common.win_dc.cur_group;
            hline_edit->common.client_dc.cur_group = p->common.client_dc.cur_group;

            /* Draw LineEdit */
            in_win_message_send_ext(hline_edit, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK);


            /* Draw btn rect */
            if ( (p->common.bimage_flag) > 0 )
            {
                rect = GET_IN_GUI_COM_BOX(p)->btn_rect;
                hdc->rect = rect;
                in_hdc_release_win(p, hdc);

                in_paint_widget_back(p);
 
                hdc = in_hdc_get_client(p);
                if ( hdc == NULL )
                    break;
 
                hdc->rect = old_rect;
                in_hdc_release_win(p, hdc);

                goto  COM_BOX_PAINT_LIST_BOX;
            }

            old_color = in_hdc_get_back_color(hdc);

            rect = GET_IN_GUI_COM_BOX(p)->btn_rect;
            hdc->rect = rect;

            if ( (p == lhdefa)&&(p != lhfocu) )
            {
                #ifdef  _LG_CURSOR_
                in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
                #endif

                in_hdc_set_back_color(hdc, DEFAULT_WINDOW_BCOLOR);
                in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

                #ifdef  _LG_CURSOR_
                in_cursor_maybe_refresh( );
                #endif
            }


            /* paint border */
            if ( IS_BORDER_WIDGET(&(p->common)) > 0 )
            {
                if ( IS_BORDER_3D_WIDGET(&(p->common)) > 0 )
                    in_paint_3d_down_border(p, &(p->common.win_dc.rect));
                else
                    in_paint_widget_border(p);
            }

            rect = GET_IN_GUI_COM_BOX(p)->btn_rect;
            hdc->rect = rect;        
            in_hdc_set_back_color(hdc, GUI_LIGHT_GRAY);

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
            #endif

            if ( (GET_IN_GUI_COM_BOX(p)->open_dir) == CBBOX_OPEN_DOWN )
            {
                in_fill_triangle(hdc, 5, 5, GUI_RECTW(&rect)/2 - 1, GUI_RECTH(&rect)-1-5, TRIANGLE_DOWN);
                in_fill_triangle(hdc, GUI_RECTW(&rect)-1-5, 5, GUI_RECTW(&rect)/2 - 1, GUI_RECTH(&rect)-1-5, TRIANGLE_DOWN);
            } else {
                in_fill_triangle(hdc, 5, GUI_RECTH(&rect)-1-5, GUI_RECTW(&rect)/2, 5, TRIANGLE_UP);
                in_fill_triangle(hdc, GUI_RECTW(&rect)-1-5, GUI_RECTH(&rect)-1-5, GUI_RECTW(&rect)/2, 5, TRIANGLE_UP);
            }

            in_hdc_set_back_color(hdc, old_color);
            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_refresh( );
            #endif

            COM_BOX_PAINT_LIST_BOX:
            /* Paint ListBox */
            if ( (GET_IN_GUI_COM_BOX(p)->state) > 0 )
                in_win_message_send_ext(hlist_box, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK);

            p->common.invalidate_flag = 0;

            in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_APP_CALLBACK);
            return  0;

        case  MSG_LOST_FOCUS:
            GET_IN_GUI_COM_BOX(p)->state = 0;
            break;

        #ifdef  _LG_CARET_
        case  MSG_CARET:
            ((GUI_MESSAGE *)msg)->to_hwnd = (void *)hline_edit;
            in_callback_to_hwnd_message(msg);
            return  0;
        #endif

        case  MSG_KEY_DOWN:
            key_value = MESSAGE_GET_KEY_VALUE(msg);

            /* GUI_KEY_HOME */
            if ( key_value != GUI_KEY_HOME )
                goto  CBBOX_GUI_KEY_END;

            if (GET_IN_GUI_COM_BOX(p)->state < 1)
                return  0;

            GET_IN_GUI_COM_BOX(p)->state = 0;

            p->common.win_dc.rect     = GET_IN_GUI_COM_BOX(p)->closed_win_rect; 
            p->common.client_dc.rect  = GET_IN_GUI_COM_BOX(p)->closed_cli_rect; 
            in_win_invalidate_area_abs(&(hlist_box->common.win_dc.rect));
            return  0;

            /* GUI_KEY_END */
            CBBOX_GUI_KEY_END:
            if ( key_value != GUI_KEY_END )
                goto  CBBOX_GUI_KEY_OTHER;

            if (GET_IN_GUI_COM_BOX(p)->state > 0)
                return  0;

            GET_IN_GUI_COM_BOX(p)->state = 1;

            p->common.win_dc.rect     = GET_IN_GUI_COM_BOX(p)->opened_win_rect; 
            p->common.client_dc.rect  = GET_IN_GUI_COM_BOX(p)->opened_cli_rect;

            in_win_paint_invalidate_recursion(lhlist);
            in_win_message_send_ext(hlist_box, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK);
            return  0;

            CBBOX_GUI_KEY_OTHER:
            if ((GET_IN_GUI_COM_BOX(p)->state) > 0)
                ((GUI_MESSAGE *)msg)->to_hwnd = (void *)hlist_box;
            else
                ((GUI_MESSAGE *)msg)->to_hwnd = (void *)hline_edit;

            in_callback_to_hwnd_message(msg);

            /* ??  */
            in_win_message_send_ext(p, MSG_NOTIFY_SEL_CHANGED, HWND_APP_CALLBACK);
            return  0;

        #ifdef   _LG_MTJT_
        case  MSG_MTJT_LBUTTON_UP:
            hdc = in_hdc_get_client(p);
            if ( hdc == NULL )
                break;

            rect = hdc->rect;     
            old_rect = rect;

            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
            {
                hdc->rect = old_rect;
                in_hdc_release_win(p, hdc);
                break; 
            }

            /* Clicked LineEdit */
            rect = hline_edit->common.client_dc.rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  CBBOX_LBUTTON_LBOX;

            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            ((GUI_MESSAGE *)msg)->to_hwnd = (void *)hline_edit;
            in_callback_to_hwnd_message(msg);
            return  0;

            /* Clicked ListBox */
            CBBOX_LBUTTON_LBOX:
            if ( (GET_IN_GUI_COM_BOX(p)->state) < 1 )
                goto  CBBOX_LBUTTON_BTN;

            rect = hlist_box->common.client_dc.rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
                goto  CBBOX_LBUTTON_BTN;

            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc);

            ((GUI_MESSAGE *)msg)->to_hwnd = (void *)hlist_box;
            in_callback_to_hwnd_message(msg);
            return  0;

            /* Clicked Button */
            CBBOX_LBUTTON_BTN:
            rect  = GET_IN_GUI_COM_BOX(p)->btn_rect;
            if ( (x < rect.left) || (x > rect.right) || (y < rect.top) || (y > rect.bottom) )
            {
                hdc->rect = old_rect;
                in_hdc_release_win(p, hdc);
                break;
            }

            hdc->rect = old_rect;
            in_hdc_release_win(p, hdc); 

            if ( (GET_IN_GUI_COM_BOX(p)->state) < 1 ) 
            {
                GET_IN_GUI_COM_BOX(p)->state = 1;

                p->common.win_dc.rect     = GET_IN_GUI_COM_BOX(p)->opened_win_rect; 
                p->common.client_dc.rect  = GET_IN_GUI_COM_BOX(p)->opened_cli_rect;

                in_win_paint_invalidate_recursion(lhlist);
                in_win_message_send_ext(hlist_box, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK);

            } else {
                GET_IN_GUI_COM_BOX(p)->state = 0;

                p->common.win_dc.rect     = GET_IN_GUI_COM_BOX(p)->closed_win_rect; 
                p->common.client_dc.rect  = GET_IN_GUI_COM_BOX(p)->closed_cli_rect; 

                in_win_invalidate_area_abs(&(hlist_box->common.win_dc.rect));
            }

            /* ?? */
            in_win_message_send_ext(p, MSG_NOTIFY_SEL_CHANGED, HWND_APP_CALLBACK);
            return  0;
        #endif
 
        default:
            break;
    }

    return  in_common_widget_callback(msg);
}

/* ComBox internal delete callback */
static  int  in_com_box_delete_callback(/* HWND */ void *hwnd)
{
    HWND   p          = hwnd;
    HWND   hline_edit = NULL;
    HWND   hlist_box  = NULL;
    int    ret        = 0;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != COM_BOX_WIDGET_TYPE)
        return  -1;


    ret = in_com_box_get_line_edit_hwnd(p, &hline_edit);
    if ( ret < 1 )
        return  -1; 

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1;
   (hlist_box->common.delete_callback)(hlist_box);

    free(hlist_box);
    hlist_box = NULL;
    free(hline_edit);
    hline_edit = NULL;
 
    return  1;
}

HWND in_com_box_create(HWND parent, void *gui_common_widget, void *gui_com_box)
{
    HWND               p = NULL;
    GUI_COM_BOX       *com_box = (GUI_COM_BOX *)gui_com_box;
    IN_GUI_COM_BOX     in_com_box;

    GUI_LINE_EDIT      line_edit;
    GUI_LIST_BOX       list_box;

    GUI_COMMON_WIDGET  tmp_com_widget; 

    GUI_RECT           rect;
    HWND               tmp_p = NULL;
    unsigned int       size = 0;
    int                ret;
   

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
    ret = in_set_hwnd_common1(parent, COM_BOX_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;


    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = CBBOX_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = CBBOX_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = CBBOX_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = CBBOX_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = CBBOX_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = CBBOX_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc                         = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = CBBOX_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = CBBOX_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = CBBOX_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = CBBOX_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = CBBOX_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = CBBOX_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = CBBOX_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = CBBOX_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = CBBOX_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = CBBOX_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = CBBOX_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = CBBOX_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc                    = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.in_callback         = in_com_box_callback;
    ltcomm.is_delete_callback  = 1;
    ltcomm.delete_callback     = in_com_box_delete_callback; 


    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_COM_BOX);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */
    memset(&in_com_box, 0, sizeof(in_com_box));

    if ( com_box != NULL )
    {
        in_com_box.line_edit_style     = com_box->line_edit_style;
        in_com_box.line_edit_ext_style = com_box->line_edit_ext_style;

        in_com_box.list_box_style      = com_box->list_box_style;
        in_com_box.list_box_ext_style  = com_box->list_box_ext_style;

        in_com_box.open_dir            = com_box->open_dir;
        in_com_box.open_len            = com_box->open_len;
    } else {

        in_com_box.line_edit_style     = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE;
        in_com_box.line_edit_ext_style = 0;

        in_com_box.list_box_style      = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE;
        in_com_box.list_box_ext_style  = 0;

        in_com_box.open_dir            = CBBOX_OPEN_DOWN;
        in_com_box.open_len            = CBBOX_LIST_BOX_HEIGHT;
    }

    if ( in_com_box.open_len == 0 )
        in_com_box.open_len = CBBOX_LIST_BOX_HEIGHT;

    if ( in_com_box.open_len < CBBOX_LIST_BOX_MINI_HEIGHT )
        in_com_box.open_len = CBBOX_LIST_BOX_MINI_HEIGHT;


    /* LineEdit */
    rect                         = ltcomm.client_dc.rect;
    /* btn width offset */
    rect.right                  -= GUI_RECTH(&rect);

    memset(&tmp_com_widget, 0, sizeof(tmp_com_widget));
    tmp_com_widget.id            = 0;
    tmp_com_widget.left          = 0;
    tmp_com_widget.right         = GUI_RECTW(&rect) - 1;
    tmp_com_widget.top           = 0;
    tmp_com_widget.bottom    = tmp_com_widget.top + GUI_RECTH(&rect) - 1;

    /* mark */
    size = GUI_RECTH(&rect);

    tmp_com_widget.style         = in_com_box.line_edit_style;
    tmp_com_widget.ext_style     = in_com_box.line_edit_ext_style;
    tmp_com_widget.acc_hwnd_flag = 1;

    memset(&line_edit, 0, sizeof(GUI_LINE_EDIT));
    line_edit.len = 0;

    tmp_p = (HWND)in_line_edit_create(p, (void *)(&tmp_com_widget), (void *)(&line_edit));
    if ( tmp_p == NULL )
        return  NULL;

    /* Importance */
    tmp_p->common.acc_hwnd_flag = 1;
    in_com_box.pline_edit       = tmp_p;


    /* ListBox */
    /* Notice: the value of ltcomm maybe changed */
    rect = p->common.client_dc.rect;
    memset(&tmp_com_widget, 0, sizeof(tmp_com_widget));
    tmp_com_widget.id            = 1;
    tmp_com_widget.left          = 0;
    tmp_com_widget.right         = GUI_RECTW(&rect) - 1;

    if ( (in_com_box.open_dir) == CBBOX_OPEN_DOWN )
        tmp_com_widget.top       = GUI_RECTH(&rect);
    else {
        tmp_com_widget.top       = -in_com_box.open_len ;
    }

    tmp_com_widget.bottom    = tmp_com_widget.top + in_com_box.open_len - 1;

    tmp_com_widget.style         = in_com_box.list_box_style;
    tmp_com_widget.ext_style     = in_com_box.list_box_ext_style;
    tmp_com_widget.acc_hwnd_flag = 1;

    memset(&list_box, 0, sizeof(list_box));
    list_box.multi_flag = 0;

    tmp_p = (HWND)in_list_box_create(p, &tmp_com_widget,  &list_box);
    if ( tmp_p == NULL )
    {
        /* ?? free line_edit hwnd */
        return  NULL;
    }
    /* Importace */
    tmp_p->common.acc_hwnd_flag  = 1;
    in_com_box.plist_box         = tmp_p;

    /* Btn_rect */
    in_com_box.btn_rect          = p->common.client_dc.rect;
    in_com_box.btn_rect.left     = in_com_box.btn_rect.right - GUI_RECTH(&(in_com_box.btn_rect)) + 1;

    /* Status */
    in_com_box.state = 0;

    /* closed and opend rect */
    in_com_box.closed_win_rect  = p->common.win_dc.rect;
    in_com_box.closed_cli_rect  = p->common.client_dc.rect;

    in_com_box.opened_win_rect = p->common.win_dc.rect;
    in_com_box.opened_cli_rect = p->common.client_dc.rect;
    if ( (in_com_box.open_dir) == CBBOX_OPEN_DOWN )
    {
        in_com_box.opened_win_rect.bottom  += in_com_box.open_len;
        in_com_box.opened_cli_rect.bottom  += in_com_box.open_len;
    } else {
        in_com_box.opened_win_rect.top     -= in_com_box.open_len;
        in_com_box.opened_cli_rect.top     -= in_com_box.open_len;
    }

    /* Copy Ext */
    memcpy(p->ext, &in_com_box, sizeof(IN_GUI_COM_BOX));


    /* 
     * add hwnd and post message.
     */
    ret = in_deal_add_hwnd(gui_common_widget, p, 0);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  com_box_create(HWND parent, void *gui_common_widget, void *gui_com_box)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_com_box_create(parent, gui_common_widget, gui_com_box);
    gui_unlock( );

    return  p;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_com_box_get_line_edit_hwnd(HWND hwnd, HWND *hline_edit)
{
    HWND  p = hwnd;

    if ( p == NULL )
        return  -1;
    if ((p->common.type) != COM_BOX_WIDGET_TYPE)
        return  -1;
 
 
    if ( hline_edit  == NULL )
         return  -1;

    *hline_edit = GET_IN_GUI_COM_BOX(p)->pline_edit;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_get_line_edit_hwnd(HWND hwnd, HWND *hline_edit)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_get_line_edit_hwnd(hwnd, hline_edit);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_com_box_get_list_box_hwnd(HWND hwnd, HWND *hlist_box)
{
    HWND  p = hwnd;

    if ( p == NULL )
        return  -1;
    if ((p->common.type) != COM_BOX_WIDGET_TYPE)
        return  -1;


    if ( hlist_box == NULL )
         return  -1;

    *hlist_box = GET_IN_GUI_COM_BOX(p)->plist_box;
 
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_get_list_box_hwnd(HWND hwnd, HWND *hlist_box)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_get_list_box_hwnd(hwnd, hlist_box);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_com_box_get_state(HWND hwnd, unsigned int *opened)
{
    HWND  p = hwnd;

    if ( p == NULL )
        return  -1;
    if ((p->common.type) != COM_BOX_WIDGET_TYPE)
        return  -1;


    if ( opened == NULL )
        return  -1;

    *opened = GET_IN_GUI_COM_BOX(p)->state;
 
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_get_state(HWND hwnd, unsigned int *opened)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_get_state(hwnd, opened);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_com_box_set_state(HWND hwnd, unsigned int opened)
{
    HWND  p = hwnd;


    if ( p == NULL )
        return  -1;
    if ((p->common.type) != COM_BOX_WIDGET_TYPE)
        return  -1;


    if ( opened > 0 ) {
        GET_IN_GUI_COM_BOX(p)->state = 1;

        p->common.win_dc.rect     = GET_IN_GUI_COM_BOX(p)->opened_win_rect; 
        p->common.client_dc.rect  = GET_IN_GUI_COM_BOX(p)->opened_cli_rect;
        in_win_paint_invalidate_recursion(lhlist);
        in_win_message_send_ext(p, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK);

        return  1;
    } else {
        GET_IN_GUI_COM_BOX(p)->state = 0;

        in_win_invalidate_area_abs(&(p->common.win_dc.rect));

        p->common.win_dc.rect     = GET_IN_GUI_COM_BOX(p)->closed_win_rect; 
        p->common.client_dc.rect  = GET_IN_GUI_COM_BOX(p)->closed_cli_rect;
         
        /* return 1; */   /* Avoid compiling warning */

    }

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_set_state(HWND hwnd, unsigned int opened)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_set_state(hwnd, opened);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_com_box_set_index_item(HWND hwnd, unsigned int index)
{
    HWND           p = hwnd;
    HWND  hline_edit = NULL;
    HWND  hlist_box  = NULL;
    unsigned int len = 0;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_line_edit_hwnd(p, &hline_edit);
    if ( ret < 1 )
        return  -1; 

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1;

    memset(glebuf, 0, sizeof(glebuf));
    len = MAX_LINE_EDIT_TEXT_LEN;
    ret = in_list_box_get_item(hlist_box, index, glebuf, &len);
    ret = in_line_edit_set_text(hline_edit, glebuf, len);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_set_index_item(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_set_index_item(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */



/* 
 *  The second fuction
 */    
int  in_com_box_line_edit_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len)
{
    HWND           p = hwnd;
    HWND  hline_edit = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;
    if ( text == NULL )
        return  -1;


    ret = in_com_box_get_line_edit_hwnd(p, &hline_edit);
    if ( ret < 1 )
        return  -1; 

    ret = in_line_edit_get_text(hline_edit, text, text_len);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_line_edit_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_line_edit_get_text(hwnd, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_com_box_line_edit_set_text(HWND hwnd, TCHAR *text, unsigned int text_len)
{
    HWND           p = hwnd;
    HWND  hline_edit = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;
    if ( text == NULL )
        return  -1;


    ret = in_com_box_get_line_edit_hwnd(p, &hline_edit);
    if ( ret < 1 )
        return  -1; 

    ret = in_line_edit_set_text(hline_edit, text, text_len);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_line_edit_set_text(HWND hwnd, TCHAR *text, unsigned int text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_line_edit_set_text(hwnd, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

 
#ifdef   _LG_COM_BOX_EXTENSION_
int  in_com_box_list_box_insert_item(HWND hwnd, int index, TCHAR  *text, unsigned int  text_len)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;
    if ( text == NULL )
        return  -1;


    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_insert_item(hlist_box, index, text, text_len);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_list_box_insert_item(HWND hwnd, int index, TCHAR  *text, unsigned int  text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_insert_item(hwnd, index, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif

int   in_com_box_list_box_get_item(HWND hwnd, unsigned int index, TCHAR *text, unsigned int *text_len)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;
    if ( text == NULL )
        return  -1;
    if ( text_len == NULL )
        return  -1;
    if ( *text_len < 1 )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_get_item(hlist_box, index, text, text_len);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_get_item(HWND hwnd, unsigned int index, TCHAR *text, unsigned int *text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_get_item(hwnd, index, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif
 
int  in_com_box_list_box_delete_item(HWND hwnd, unsigned int index)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_delete_item(hlist_box, index);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_list_box_delete_item(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_delete_item(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif
    
int   in_com_box_list_box_set_select_mode(HWND hwnd, unsigned int  mode)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_set_select_mode(hlist_box, mode);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  com_box_list_box_set_select_mode(HWND hwnd, unsigned int mode)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_set_select_mode(hwnd, mode);
    gui_unlock( );

    return  ret;
}
#endif
    
int   in_com_box_list_box_get_select_mode(HWND hwnd, unsigned int *mode)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;
    if ( mode == NULL )
        return  -1;


    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_get_select_mode(hlist_box, mode);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_get_select_mode(HWND hwnd, unsigned int *mode)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_get_select_mode(hwnd, mode);
    gui_unlock( );

    return  ret;
}
#endif
 
int   in_com_box_list_box_set_selected_index(HWND hwnd, unsigned int index)
{ 
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_set_selected_index(hlist_box, index);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_set_selected_index(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_set_selected_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif
  
int   in_com_box_list_box_is_selected_index(HWND hwnd, unsigned int index)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_is_selected_index(hlist_box, index);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_is_selected_index(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_is_selected_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif

int   in_com_box_list_box_get_selected_index(HWND hwnd, unsigned int start_index, int *index)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_get_selected_index(hlist_box, start_index, index);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_get_selected_index(HWND hwnd, unsigned int start_index, int *index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_get_selected_index(hwnd, start_index, index);
    gui_unlock( );

    return  ret;
}
#endif

int   in_com_box_list_box_set_lighted_index(HWND hwnd, unsigned int index)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_set_lighted_index(hlist_box, index);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_set_lighted_index(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_set_lighted_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif
     
int   in_com_box_list_box_is_lighted_index(HWND hwnd, unsigned int index)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_is_lighted_index(hlist_box, index);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_is_lighted_index(HWND hwnd, unsigned int index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_is_lighted_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif

int   in_com_box_list_box_get_lighted_index(HWND hwnd, int *index)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_get_lighted_index(hlist_box, index);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_get_lighted_index(HWND hwnd, int *index)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_get_lighted_index(hwnd, index);
    gui_unlock( );

    return  ret;
}
#endif
 
int   in_com_box_list_box_get_item_counter(HWND hwnd, unsigned int *counter)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_get_item_counter(hlist_box, counter);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_get_item_counter(HWND hwnd, unsigned int *counter)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_get_item_counter(hwnd, counter);
    gui_unlock( );

    return  ret;
}
#endif
    
int   in_com_box_list_box_is_read_only(HWND hwnd)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_is_read_only(hlist_box);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_is_read_only(HWND hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_is_read_only(hwnd);
    gui_unlock( );

    return  ret;
}
#endif
   
int   in_com_box_list_box_set_read_only(HWND hwnd, unsigned int read_only)
{
    HWND           p = hwnd;
    HWND  hlist_box  = NULL;
    int   ret        = 0;


    if ( p == NULL )
        return  -1;

    ret = in_com_box_get_list_box_hwnd(p, &hlist_box);
    if ( ret < 1 )
        return  -1; 

    ret = in_list_box_set_read_only(hlist_box, read_only);
    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int   com_box_list_box_set_read_only(HWND hwnd, unsigned int read_only)
{
    int  ret = 0;

    gui_lock( );
    ret = in_com_box_list_box_set_read_only(hwnd, read_only);
    gui_unlock( );

    return  ret;
}
#endif    
#endif  /* _LG_COM_BOX_EXTENSION_ */

#endif  /* _LG_COM_BOX_WIDGET_ */
#endif  /* _LG_WINDOW_ */
