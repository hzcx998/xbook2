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
#include  <win_caret.h>
#include  <win_line_edit.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_LINE_EDIT_WIDGET_

/* Line edit tmp buffer */         
static TCHAR  gleraw[MAX_LINE_EDIT_TEXT_LEN+1] = "";
static TCHAR  glebuf[MAX_LINE_EDIT_TEXT_LEN+1] = "";


/* Line Edit internal callback */
static  int  in_line_edit_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND           p     = NULL;
    HWND           tmp_p = NULL;
    HDC            hdc;
    GUI_RECT       rect;
    GUI_COLOR      old_color;
    unsigned int   text_len  = 0;
             int   key_value = 0;

 
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
                    in_paint_3d_down_border(p, &(p->common.win_dc.rect));
                else
                    in_paint_widget_border(p);
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
 
            in_text_out_rect(hdc, &rect, ((IN_GUI_LINE_EDIT *)(p->ext))->text, -1, LG_TA_VCENTER);

            in_hdc_set_back_color(hdc, old_color);
            in_hdc_release_win(p, hdc);

            p->common.invalidate_flag = 0;

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_refresh( );
            #endif
 
            in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_APP_CALLBACK);
            break;

        case  MSG_GET_FOCUS:
            text_len = strlen(((IN_GUI_LINE_EDIT *)(p->ext))->text);
            memset(gleraw, 0, sizeof(gleraw));
            memcpy(gleraw, ((IN_GUI_LINE_EDIT *)(p->ext))->text, text_len);
            break;

        case  MSG_LOST_FOCUS:
            text_len = strlen(gleraw);
            memset(((IN_GUI_LINE_EDIT *)(p->ext))->text, 0, sizeof(((IN_GUI_LINE_EDIT *)(p->ext))->text));
            memcpy(((IN_GUI_LINE_EDIT *)(p->ext))->text, gleraw, text_len);
            /* Rao 20180402 del */
            /* in_win_message_send_ext(p, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK); */
            break;

        case  MSG_KEY_DOWN:
            text_len = strlen(((IN_GUI_LINE_EDIT *)(p->ext))->text);
            #ifdef  _LG_CARET_
            if ( lcarin > text_len )
                lcarin = text_len;
            #endif
            key_value = MESSAGE_GET_KEY_VALUE(msg);
            if ( (key_value < GUI_KEY_FIRST_CHAR) || (key_value > GUI_KEY_END_CHAR) )
                goto  LEDIT_CB_KEY_MOVE;

            #ifdef  _LG_CARET_
            memset(glebuf, 0, sizeof(glebuf));
            strncat(glebuf, ((IN_GUI_LINE_EDIT *)(p->ext))->text, lcarin );
            strncat(glebuf, (const char *)(&key_value), 1);
            strncat(glebuf, ((IN_GUI_LINE_EDIT *)(p->ext))->text + lcarin, text_len - lcarin );

            memset(((IN_GUI_LINE_EDIT *)(p->ext))->text, 0, sizeof(glebuf));
            strcat(((IN_GUI_LINE_EDIT *)(p->ext))->text, glebuf );
            lcarin++;
            #else
            strncat(((IN_GUI_LINE_EDIT *)(p->ext))->text, &key_value, 1);
            #endif

            in_win_message_send_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            in_win_message_send_ext(p, MSG_CARET, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            break;

            LEDIT_CB_KEY_MOVE:
            #ifdef  _LG_CARET_
            if ( (key_value < GUI_KEY_FIRST_MOVE) || (key_value > GUI_KEY_END_MOVE) )
                goto  LEDIT_CB_KEY_DELETE;

            if ( (key_value == GUI_KEY_RIGHT) || (key_value == GUI_KEY_DOWN))
            {
                if ( lcarin < text_len )
                    lcarin++;
            } else if ( (key_value == GUI_KEY_LEFT) || (key_value == GUI_KEY_UP) ) {
                if ( lcarin != 0 )
                    lcarin--;
            } else if ( (key_value == GUI_KEY_HOME) || (key_value == GUI_KEY_PAGE_UP) ) {
                 lcarin  = 0;
            } else if ( (key_value == GUI_KEY_END) || (key_value == GUI_KEY_PAGE_DOWN) ) {
                lcarin  = text_len;
            }

            in_win_message_send_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            in_win_message_send_ext(p, MSG_CARET, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            break;
            #endif

            LEDIT_CB_KEY_DELETE:
            if ( key_value != GUI_KEY_DELETE ) 
                goto  LEDIT_CB_KEY_BACKSPACE;

            memset(glebuf, 0, sizeof(glebuf));

            #ifdef  _LG_CARET_
            if ( lcarin == text_len )
                break;
            strncat(glebuf, ((IN_GUI_LINE_EDIT *)(p->ext))->text, lcarin );
            strncat(glebuf, ((IN_GUI_LINE_EDIT *)(p->ext))->text + lcarin + 1, text_len - lcarin - 1 );
            #else
            strncat(glebuf, ((IN_GUI_LINE_EDIT *)(p->ext))->text, text_len-1 );
            #endif

            memset(((IN_GUI_LINE_EDIT *)(p->ext))->text, 0, sizeof(glebuf));
            strcat(((IN_GUI_LINE_EDIT *)(p->ext))->text, glebuf );

            in_win_message_send_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            in_win_message_send_ext(p, MSG_CARET, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            break;

 
            LEDIT_CB_KEY_BACKSPACE:
            if ( key_value != GUI_KEY_BACKSPACE ) 
                goto  LEDIT_CB_KEY_ENTER;

            memset(glebuf, 0, sizeof(glebuf));

            #ifdef  _LG_CARET_
            if ( lcarin == 0 )
                break;
            strncat(glebuf, ((IN_GUI_LINE_EDIT *)(p->ext))->text, lcarin-1 );
            strncat(glebuf, ((IN_GUI_LINE_EDIT *)(p->ext))->text + lcarin, text_len - lcarin );
            lcarin--;
            #else
            strncat(glebuf, ((IN_GUI_LINE_EDIT *)(p->ext))->text, text_len-1 );
            #endif

            memset(((IN_GUI_LINE_EDIT *)(p->ext))->text, 0, sizeof(glebuf));
            strcat(((IN_GUI_LINE_EDIT *)(p->ext))->text, glebuf );

            in_win_message_send_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            in_win_message_send_ext(p, MSG_CARET, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
            break;

            LEDIT_CB_KEY_ENTER:
            if ( (key_value != GUI_KEY_ENTER) ) 
                break;

            memset(gleraw, 0, sizeof(gleraw));
            memcpy(gleraw, ((IN_GUI_LINE_EDIT *)(p->ext))->text, text_len);
            /* ?? */
            /*    */
            break;

        #ifdef  _LG_CARET_
        case  MSG_CARET:
            if ( in_win_is_visual(p) < 1 )
                break;

            text_len = strlen(((IN_GUI_LINE_EDIT *)(p->ext))->text);

            #ifdef  _LG_CARET_
            if ( lcarin > text_len )
                lcarin = text_len;
            #endif

            if ( lcarfl < 1 )
                goto  LEDIT_CB_HIDE_CARET;

            /* Show caret */

            /* Fix me ?? */
            hdc      = in_hdc_get_client(p);
            if ( hdc == NULL )
                break;

            rect.left   = lcarin*8;
            rect.top    = 2;
            rect.right  = rect.left + lcarwi;
            rect.bottom = GUI_RECTH(&(hdc->rect))-1-2; 

            lcarre = rect;

            old_color = in_hdc_get_back_color(hdc);
            in_hdc_set_back_color(hdc, CARET_FORE_COLOR);
            in_rect_fill(hdc, rect.left, rect.top, rect.right, rect.bottom);
            in_hdc_set_back_color(hdc, old_color);
            in_hdc_release_win(p, hdc);

            lcarfl = 0;
            break;

            /* Hide caret */
            LEDIT_CB_HIDE_CARET:
            tmp_p = p;

            /* Difference ?? */
            /* Fix me */
            if ( (p->common.acc_hwnd_flag) > 0 )
                tmp_p = p->head.parent;

            in_win_invalidate_rect(tmp_p, (const GUI_RECT *)(&lcarre));

            lcarfl = 1;
            return  0;
        #endif

        default:
            break;
    }

    return  in_common_widget_callback(msg);
}

HWND in_line_edit_create(HWND parent, void *gui_common_widget, void *gui_line_edit)
{
    HWND                p  = NULL;
    GUI_LINE_EDIT      *line_edit     = (GUI_LINE_EDIT *)gui_line_edit;
    IN_GUI_LINE_EDIT    in_line_edit;
    unsigned int        size = 0;
    int                 ret;
   

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
    ret = in_set_hwnd_common1(parent, LINE_EDIT_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;

    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = LEDIT_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = LEDIT_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = LEDIT_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = LEDIT_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = LEDIT_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = LEDIT_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc                         = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = LEDIT_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = LEDIT_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = LEDIT_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = LEDIT_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = LEDIT_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = LEDIT_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = LEDIT_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = LEDIT_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = LEDIT_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = LEDIT_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = LEDIT_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = LEDIT_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc                    = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.in_callback = in_line_edit_callback;


    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_LINE_EDIT);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */
    memset(&in_line_edit, 0, sizeof(in_line_edit));
    if ( line_edit != NULL )
        in_line_edit = *line_edit;

     memcpy(p->ext, &in_line_edit, sizeof(IN_GUI_LINE_EDIT));


    /* 
     * add hwnd and post message.
     */
    /* ?? */
    ret = in_deal_add_hwnd(gui_common_widget, p, ((GUI_COMMON_WIDGET *)gui_common_widget)->acc_hwnd_flag);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  line_edit_create(HWND parent, void *gui_common_widget, void *gui_line_edit)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_line_edit_create(parent, gui_common_widget, gui_line_edit);
    gui_unlock( );

    return  p;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_line_edit_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len)
{
    HWND  p = hwnd;
    unsigned int  line_edit_len = 0;
    unsigned int  copy_len      = 0;


    if ( p == NULL )
        return  -1;
    if ( text == NULL )
        return  -1;
    if ( *text_len < 1 )
        return  -1;

    if ((p->common.type) != LINE_EDIT_WIDGET_TYPE)
        return  -1;

    line_edit_len = ((IN_GUI_LINE_EDIT *)(p->ext))->len;
    copy_len      = (*text_len > line_edit_len ? line_edit_len : *text_len);
    memcpy(text, ((IN_GUI_LINE_EDIT *)(p->ext))->text, copy_len);
    *text_len = copy_len;         

    return  copy_len;
}

#ifndef  _LG_ALONE_VERSION_
int  line_edit_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_line_edit_get_text(hwnd, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_line_edit_set_text(HWND hwnd, TCHAR *text, unsigned int text_len)
{
    HWND  p = hwnd;
    unsigned int  copy_len = 0;


    if ( p == NULL )
        return  -1;
    if ( text == NULL )
        return  -1;
    if ( text_len < 1 )
        return  -1;

    if ((p->common.type) != LINE_EDIT_WIDGET_TYPE)
        return  -1;

    copy_len = (text_len > MAX_LINE_EDIT_TEXT_LEN ? MAX_LINE_EDIT_TEXT_LEN : text_len);
    memset(((IN_GUI_LINE_EDIT *)(p->ext))->text, 0, MAX_LINE_EDIT_TEXT_LEN);
    memcpy(((IN_GUI_LINE_EDIT *)(p->ext))->text, text, copy_len); 

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  line_edit_set_text(HWND hwnd, TCHAR *text, unsigned int text_len)
{
    int  ret = 0;

    gui_lock( );
    ret = in_line_edit_set_text(hwnd, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_LINE_EDIT_WIDGET_ */
#endif  /* _LG_WINDOW_ */
