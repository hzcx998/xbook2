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

#include  <string.h>
#include  <stdlib.h>

#include  <lgconst.h>

#include  <lock.h>
#include  <cursor.h>

#include  <default.h>

#include  <d2_line.h>

#include  <d2_rect.h>
#include  <image_comm.h>
#include  <image_bitmap.h>
#include  <image_icon.h>
#include  <image_gif.h>

#include  <win_type_widget.h>
#include  <win_tools.h>
#include  <win_callback.h>
#include  <win_invalidate.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_invalidate.h>
#include  <win_widget.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"



#ifdef  _LG_WINDOW_
int  in_set_hwnd_common1(/*HWND*/ void *parent, unsigned int widget_type,  /* GUI_COMMMON_WIDGET */ void *gui_common_widget)
{
    GUI_COMMON_WIDGET  *common_widget = (GUI_COMMON_WIDGET *)gui_common_widget;
    GUI_RECT            rect;
    int                 delta;


    if ( common_widget == NULL )
        return  -1;


    /* Ensure that it's a stardard rectangle */
    if ( common_widget->left > common_widget->right )
        return  -1;
    if ( common_widget->top  > common_widget->bottom )
        return  -1;


    /* set parent */
    memset((void *)(&lthead), 0, sizeof(GUI_WND_HEAD));

    lthead.prev    = NULL;
    lthead.next    = NULL;
    lthead.parent  = parent;
    lthead.fc      = NULL;
    lthead.lc      = NULL;


    /* set ltcomm */
    memset((void *)(&ltcomm), 0, sizeof(GUI_COMMON_WIDGET));

    /* Set id */
    ltcomm.id                    = common_widget->id;

    /* Widget type */    
    ltcomm.type                  = widget_type;

    /* Style and ext style */
    /* ltcomm.style              = (common_widget->style) & WIDGET_STYLE_MASK;  */
    ltcomm.style                 = common_widget->style; 
    ltcomm.ext_style             = common_widget->ext_style;


    /* Widget border width */
    if ( win_border_width < IN_MIN_BORDER_WIDTH )
        win_border_width = IN_MIN_BORDER_WIDTH;
 
    ltcomm.border_width          = win_border_width;


    /* Set window DC */
    /* ?? */
    memset((void *)(&ltdc), 0, sizeof(GUI_DC));
    in_hdc_basic_set_default((HDC)(&ltdc));
    in_hdc_window_set_default((HDC)(&ltdc));

    memset(&rect, 0, sizeof(rect));
    rect.left                    = common_widget->left;
    rect.top                     = common_widget->top;
    rect.right                   = common_widget->right;
    rect.bottom                  = common_widget->bottom;
    if (lthead.parent != NULL)
    {
        delta = (lthead.parent)->common.client_dc.rect.left;
        rect.left   += delta;
        rect.right  += delta;

        delta = (lthead.parent)->common.client_dc.rect.top;
        rect.top    += delta;
        rect.bottom += delta;
    }
    ltdc.rect                             = rect;
    ltdc.font                             = (GUI_FONT *)lwdcft;

    return  1;
}

int  in_set_hwnd_common2(/* GUI_COMMMON_WIDGET */ void *gui_common_widget)
{
    GUI_COMMON_WIDGET  *common_widget = (GUI_COMMON_WIDGET *)gui_common_widget;
    GUI_RECT            rect;


    if ( common_widget == NULL )
        return  -1;

    /* Client rect */
    ltdc.font                             = (GUI_FONT *)lcdcft;
    rect                                  = ltcomm.win_dc.rect;
    if ( IS_BORDER_WIDGET(&ltcomm) )
    {
        rect.left                        += win_border_width;
        rect.right                       -= win_border_width;
        rect.top                         += win_border_width;
        rect.bottom                      -= win_border_width;
    }
    #ifdef  _LG_WINDOW_BAR_
    if (IS_WINBAR_WIDGET(&ltcomm) > 0 )
        rect.top                         += win_bar_height;
    #endif

    ltdc.rect                             = rect;

    in_hdc_client_set_default((HDC)(&ltdc));

    return  1;
}

int  in_set_hwnd_common3(/* GUI_COMMMON_WIDGET */ void *gui_common_widget)
{
    GUI_COMMON_WIDGET  *common_widget = (GUI_COMMON_WIDGET *)gui_common_widget;
    GUI_RECT            rect;


    if ( common_widget == NULL )
        return  -1;


    /* ?? */
    /* Window bar */
    #ifdef  _LG_WINDOW_BAR_
    ltcomm.winbar = NULL;
    if (IS_WINBAR_WIDGET(&ltcomm) < 1 )
        goto  WBAR_END;
 
    memset((void *)(&ltwbar), 0, sizeof(GUI_WINBAR));

    if ( win_bar_height < IN_MIN_WINDOW_BAR_HEIGHT )
        win_bar_height = IN_MIN_WINDOW_BAR_HEIGHT;

    if ( win_system_button_width < IN_MIN_SYSTEM_BUTTON_WIDTH )
        win_system_button_width = IN_MIN_SYSTEM_BUTTON_WIDTH;

    rect                         = ltcomm.win_dc.rect;
    ltwbar.status                = WINDOW_NORMAL_STATUS;
    ltwbar.raw_rect              = rect;
    if (IS_BORDER_WIDGET(&ltcomm) > 0 )
    {
        rect.left               += win_border_width;
        rect.right              -= win_border_width;
        rect.top                += win_border_width;
    }
    rect.bottom                  = rect.top + (win_bar_height - 1);
    ltwbar.bar_rect              = rect;
    ltwbar.title_rect            = ltwbar.bar_rect;
    ltwbar.title_rect.right     -= IN_SYSTEM_BUTTON_H_DISTANCE;

    ltwbar.close_rect.left       = -1;
    ltwbar.close_rect.right      = -1;
    ltwbar.close_rect.top        = -1;
    ltwbar.close_rect.bottom     = -1;

    ltwbar.max_rect.left         = -1;
    ltwbar.max_rect.right        = -1;
    ltwbar.max_rect.top          = -1;
    ltwbar.max_rect.bottom       = -1;

    ltwbar.min_rect.left         = -1;
    ltwbar.min_rect.right        = -1;
    ltwbar.min_rect.top          = -1;
    ltwbar.min_rect.bottom       = -1;

    rect.right                  -= IN_SYSTEM_BUTTON_H_DISTANCE;
    rect.top                    += IN_SYSTEM_BUTTON_VT_DISTANCE;
    rect.bottom                 -= IN_SYSTEM_BUTTON_VB_DISTANCE;

    if ( IS_CLOSE_BTN_WINBAR(&ltcomm) > 0 )
    {
        ltwbar.close_rect               = rect;
        ltwbar.close_rect.left          = ltwbar.close_rect.right - (win_system_button_width-1);
    } 

    if ( IS_MAX_BTN_WINBAR(&ltcomm) > 0 )
    {
        ltwbar.max_rect                 = rect;
        if ( IS_CLOSE_BTN_WINBAR(&ltcomm) > 0 )
            ltwbar.max_rect.right      -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

        ltwbar.max_rect.left            = ltwbar.max_rect.right - (win_system_button_width-1);
    }

    if ( IS_MIN_BTN_WINBAR(&ltcomm) > 0 )
    {
        ltwbar.min_rect                 = rect;
        if ( IS_CLOSE_BTN_WINBAR(&ltcomm) > 0 )
            ltwbar.min_rect.right      -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

        if ( IS_MAX_BTN_WINBAR(&ltcomm) > 0 )
            ltwbar.min_rect.right      -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

        ltwbar.min_rect.left            = ltwbar.min_rect.right - (win_system_button_width-1);
    }

    WBAR_END:
    #endif

    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
    /* Background image */
    ltcomm.bimage_flag    = common_widget->bimage_flag;
    ltcomm.bimage_type    = common_widget->bimage_type;
    ltcomm.bimage_align   = common_widget->bimage_align;
    ltcomm.pimage         = common_widget->pimage;
    #endif

    #ifdef  _LG_WIDGET_USER_DATA_
    /* User data max bytes */
    if ( (common_widget->max_data_bytes) > MAX_USER_DATA_LEN )
    {
        common_widget->max_data_bytes = MAX_USER_DATA_LEN;
    }
    ltcomm.max_data_bytes   = common_widget->max_data_bytes; 

    /* User data bytes */
    if ( ( common_widget->data_bytes) > (common_widget->max_data_bytes) )
    {
        common_widget->data_bytes = common_widget->max_data_bytes;
    } 
    ltcomm.data_bytes       = common_widget->data_bytes; 
    #endif  /* _LG_WIDGET_USER_DATA_ */

    /* Callback function */
    ltcomm.no_focus_flag         = 0;
    ltcomm.no_erase_back_flag    = 0;
    ltcomm.is_in_callback        = 1;

    ltcomm.is_delete_callback    = 0;
    ltcomm.delete_callback       = NULL; 

    ltcomm.is_app_callback       = common_widget->is_app_callback;
    ltcomm.app_callback          = common_widget->app_callback; 

    return  1;
}

/*
 * HWND data structure orginization:
 * 1. Head pointer zone          (head)
 * 2. Common_widget zone         (common_widget)
 * 3. widget special zone        (ext0_widget)
 * 4. Wbar zone                  (ext1_widget)
 * 5. Hbar zone                  (ext2_widget)
 * 6. Vbar zone                  (ext3_widget)
 * 7. User data zone             (ext4_widget)
 */     
HWND  in_malloc_hwnd_memory(/* GUI_COMMMON_WIDGET */ void *gui_common_widget, unsigned int size)
{
    HWND  p = NULL;
    GUI_COMMON_WIDGET  *common_widget = (GUI_COMMON_WIDGET *)gui_common_widget;

    unsigned int  totall_size   = size;
    unsigned int  offset_schbar = 0;
    unsigned int  offset_scvbar = 0;
    unsigned int  offset_pdata  = 0;


    if ( common_widget == NULL )
        return  NULL;


    /* ?? */
    /* Align or offset */


    /* Window bar */
    #ifdef  _LG_WINDOW_BAR_
    if (IS_WINBAR_WIDGET(&ltcomm) > 0)
    {
        totall_size  += sizeof(GUI_WINBAR);

        offset_schbar = sizeof(GUI_WINBAR);
        offset_scvbar = offset_schbar;
        offset_pdata  = offset_scvbar;
    }
    #endif

    /* Scroll hbar and vbar */
    #ifdef  _LG_SCROLL_BAR_
    if (IS_HBAR_SCBAR(&ltcomm) > 0)
    {
        totall_size   += sizeof(GUI_SCBAR);
        offset_scvbar += sizeof(GUI_SCBAR);
        offset_pdata  = offset_scvbar;
    }
 
    if (IS_VBAR_SCBAR(&ltcomm) > 0)
    {
        totall_size   += sizeof(GUI_SCBAR);
        offset_pdata  += sizeof(GUI_SCBAR);
    }
    #endif

    /* User data */
    #ifdef  _LG_WIDGET_USER_DATA_
    totall_size += ltcomm.max_data_bytes; 
    #endif


    p = malloc(totall_size); 
    if ( p == NULL )
        return  NULL;
    memset(p, 0, totall_size);


    /* Copy data from user side to memory */
    memcpy(&(p->head), (const void *)(&lthead), sizeof(GUI_WND_HEAD));
    memcpy(&(p->common), (const void *)(&ltcomm), sizeof(GUI_COMMON_WIDGET));

    p->common.win_dc.hwnd     = p;
    p->common.client_dc.hwnd  = p;
    p->common.invalidate_flag = 0;


    /* Window bar */
    #ifdef  _LG_WINDOW_BAR_
    if (IS_WINBAR_WIDGET(&ltcomm) > 0)
        /* ?? */
        /* p->common.winbar = ((void *)p) + size; */
        p->common.winbar = (GUI_WINBAR *)(((char *)p) + size);
    #endif

    /* Scroll hbar and vbar */
    #ifdef  _LG_SCROLL_BAR_
    if (IS_HBAR_SCBAR(&ltcomm) > 0)
        /* ?? */
        /* p->common.schbar = ((void *)p)  + size + offset_schbar; */
        p->common.schbar = (GUI_SCBAR *)(((char *)p)  + size + offset_schbar);

    if (IS_VBAR_SCBAR(&ltcomm) > 0)
        /* ?? */
        /* p->common.scvbar = ((void *)p) + size + offset_scvbar; */
        p->common.scvbar = (GUI_SCBAR *)(((char *)p) + size + offset_scvbar);
    #endif

    /* User data */
    #ifdef  _LG_WIDGET_USER_DATA_
    /* ?? */
    /* p->common.pdata = ((void *)p) + size + offset_pdata; */
    p->common.pdata = ((char *)p) + size + offset_pdata;
    if ( (common_widget->data_bytes) > 0 )
    {
        memcpy(p->common.pdata, common_widget->pdata, 
               common_widget->max_data_bytes >= common_widget->data_bytes ? common_widget->data_bytes : common_widget->max_data_bytes);
    }
    #endif

    return  p;
}
    
int in_deal_add_hwnd(/* GUI_COMMMON_WIDGET */ void *gui_common_widget, HWND hwnd, BUINT acc_hwnd_flag)
{
    HWND   p = hwnd;
    GUI_COMMON_WIDGET  *common_widget = (GUI_COMMON_WIDGET *)gui_common_widget;
    int   ret = 0;


    if ( common_widget == NULL )
        return  -1;

    common_widget->acc_hwnd_flag = acc_hwnd_flag;
    if ( common_widget->acc_hwnd_flag > 0 )
        return  1;


    /* Add hwnd */
    ret = in_win_add(p);
    if (ret < 0)
    {
        free(p);
        p = NULL;
        return  -1;
    }

    in_win_message_post_ext(p, MSG_CREATE, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
    in_win_message_post_ext(p, MSG_CREATE_NEXT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);
	if ( ((p->head.parent) == HWND_DESKTOP) || ((p->head.parent) == NULL) )
        in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}


int  in_paint_widget_back(/* HWND */ void *hwnd)
{
    HWND             p   = (HWND)hwnd;
    HDC              hdc = NULL;
    GUI_RECT         rect;
    GUI_COLOR        old_color;
    int              left = 0;
    int              top  = 0;
    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
    const  GUI_GIF  *pgif = NULL;
    #endif


    if ( p == NULL )
        return  -1;

    if ( (p->common.no_erase_back_flag) > 0 )
        return  -1;

    hdc  = in_hdc_get_client(p);
    if ( hdc == NULL )
        return  -1;

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
    #endif

    old_color = in_hdc_get_back_color(hdc);

    rect.left   = 0;
    rect.top    = 0;
    rect.right  = GUI_RECTW(&(hdc->rect)) - 1;
    rect.bottom = GUI_RECTH(&(hdc->rect)) - 1;

    /* No background image */
    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
    if ( ( p->common.bimage_flag) > 0  )
        goto  PAINT_BIMAGE_BITMAP;
    #endif

    if ( (p == lhdefa)&&(p != lhfocu)&&(p != lhdesk) ) 
        in_hdc_set_back_color(hdc, DEFAULT_WINDOW_BCOLOR);

    in_rect_fill(hdc, rect.left, rect.top, rect.right, rect.bottom);
    goto  PAINT_BIMAGE_OK;

    
#ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
PAINT_BIMAGE_BITMAP:
    /* Bitmap background image */
    if ( (p->common.bimage_type) != IMAGE_BITMAP )
        goto  PAINT_BIMAGE_ICON;

    if ( (p->common.bimage_align) != IMAGE_ALIGN_FILL )
        goto  PAINT_BIMAGE_NO_FILL_BITMAP;

    in_bitmap_fill_rect(hdc, &rect, p->common.pimage);
    goto  PAINT_BIMAGE_OK;

PAINT_BIMAGE_NO_FILL_BITMAP:
    in_bitmap_fill(hdc, left, top, p->common.pimage);
    goto  PAINT_BIMAGE_OK;


PAINT_BIMAGE_ICON:
    /* Icon background image */
    if ( (p->common.bimage_type) != IMAGE_ICON )
        goto  PAINT_BIMAGE_GIF;

    if ( (p->common.bimage_align) != IMAGE_ALIGN_FILL )
        goto  PAINT_BIMAGE_NO_FILL_ICON;

    in_icon_fill_rect(hdc, &rect, p->common.pimage);
    goto  PAINT_BIMAGE_OK;

PAINT_BIMAGE_NO_FILL_ICON:
    in_icon_fill(hdc, left, top, p->common.pimage);
    goto  PAINT_BIMAGE_OK;


PAINT_BIMAGE_GIF:
    /* Gif background image */
    if ( (p->common.bimage_type) != IMAGE_GIF )
         goto  PAINT_BIMAGE_OK;

    pgif = p->common.pimage;
    if ( p->common.frame_id > (pgif->block_num - 1) )
        p->common.frame_id = 0;
    in_gif_man_play(hdc, rect.left, rect.top, p->common.pimage, p->common.frame_id);
    p->common.frame_id++;
#endif  /* _LG_WINDOW_BACKGROUND_IMAGE_ */


PAINT_BIMAGE_OK:
    in_hdc_set_back_color(hdc, old_color);
    in_hdc_release_win(p, hdc);
    p->common.invalidate_flag = 0;

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
 
    return  1;
}


int  in_paint_widget_border(/* HWND */ void *hwnd)
{
    HWND             p   = (HWND)hwnd;
    HDC              hdc = NULL;
    GUI_RECT         rect;
    GUI_COLOR        old_color;
    int              i    = 0;


    if ( p == NULL )
        return  -1;

    hdc  = in_hdc_get_client(p);
    if ( hdc == NULL )
        return  -1;

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
    #endif

    old_color = in_hdc_get_back_color(hdc);

    rect.left   = 0;
    rect.top    = 0;
    rect.right  = GUI_RECTW(&(hdc->rect)) - 1;
    rect.bottom = GUI_RECTH(&(hdc->rect)) - 1;

    if ( (p == lhdefa)&&(p != lhfocu)&&(p != lhdesk) ) 
        in_hdc_set_back_color(hdc, DEFAULT_WINDOW_BCOLOR);

    for ( i = 0; i < (p->common.border_width); i++ )
    {
        in_rect_frame(hdc, rect.left, rect.top, rect.right, rect.bottom);
        rect.left   += 1;
        rect.top    += 1;
        rect.right  -= 1;
        rect.bottom -= 1;
    }

    in_hdc_set_back_color(hdc, old_color);
    in_hdc_release_win(p, hdc);

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
 
    return  1;
}


int  in_paint_3d_up_border(/*HWND*/ void *hwnd, /* GUI_RECT * */ void *rect)
{
    HWND      p     = (HWND)hwnd;
    GUI_RECT  trect  = *((GUI_RECT *)rect);
    HDC       hdc;
    GUI_COLOR old_color;
    int       i;


    if (p == NULL)
        return  -1;
    if (rect == NULL)
        return  -1;

        
    in_win_set_current_color_group(p);

    hdc = in_hdc_get_window(p);
    if (hdc == NULL)
        return  -1;

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
    #endif

    trect.left   -= hdc->rect.left;
    trect.right  -= hdc->rect.left;
    trect.top    -= hdc->rect.top;
    trect.bottom -= hdc->rect.top;

    old_color = in_hdc_get_current_group_color(hdc, FORE_ROLE);

    in_hdc_set_fore_color(hdc, GUI_3D_UP_BORDER_FCOLOR);
    in_rect_frame(hdc, trect.left, trect.top, trect.right, trect.bottom);

    in_hdc_set_fore_color(hdc, GUI_3D_UP_MID_FCOLOR);
    for ( i= 0; i < 2; i++)
        in_line(hdc, trect.left + 1, trect.top + 1+i, trect.right-2, trect.top + 1+i);
    for ( i= 0; i < 2; i++)
        in_line(hdc, trect.left + 1+i, trect.top + 2, trect.left + 1+i, trect.bottom-2);
    
    in_hdc_set_fore_color(hdc, GUI_3D_UP_IN_FCOLOR);
    for ( i= 0; i < 2; i++)
        in_line(hdc, trect.left + 1, trect.bottom - 1-i, trect.right-2, trect.bottom-1-i);
    for ( i= 0; i < 2; i++)
        in_line(hdc, trect.right-1-i, trect.top + 2, trect.right-1-i, trect.bottom-1);

    in_hdc_set_current_group_color(hdc, FORE_ROLE, old_color);
    in_hdc_release_win(p, hdc);

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
 
    return  1;
}
 
int  in_paint_3d_down_border(/*HWND*/ void *hwnd, /* GUI_RECT * */ void *rect)
{
    HWND      p    = (HWND)hwnd;
    GUI_RECT  trect = *((GUI_RECT *)rect);
    HDC       hdc;
    GUI_COLOR old_color;


    if (p == NULL)
        return  -1;
    if (rect == NULL)
        return  -1;


    hdc = in_hdc_get_window(p);
    if (hdc == NULL)
        return  -1;

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
    #endif

    trect.left   -= hdc->rect.left;
    trect.right  -= hdc->rect.left;
    trect.top    -= hdc->rect.top;
    trect.bottom -= hdc->rect.top;

    old_color = in_hdc_get_current_group_color(hdc, FORE_ROLE);

    in_hdc_set_fore_color(hdc, GUI_3D_DOWN_BORDER_FCOLOR);
    in_rect_frame(hdc, trect.left, trect.top, trect.right, trect.bottom);

    in_hdc_set_fore_color(hdc, GUI_3D_DOWN_MID_FCOLOR);
    in_line(hdc, trect.left+1, trect.top+1, trect.right-2, trect.top+1);
    in_line(hdc, trect.left+1, trect.top+2, trect.left+1, trect.bottom-2);

    in_hdc_set_fore_color(hdc, GUI_3D_DOWN_BORDER_FCOLOR);
    in_line(hdc, trect.left+1, trect.top+2, trect.right-2, trect.top+2);
    in_line(hdc, trect.left+2, trect.top+2, trect.left+2, trect.bottom-2);

    in_hdc_set_fore_color(hdc, GUI_3D_DOWN_IN_FCOLOR);
    in_line(hdc, trect.right-2, trect.top+1, trect.right-2, trect.bottom-2);
    in_line(hdc, trect.left+1, trect.bottom-2, trect.right-2, trect.bottom-2);

    in_hdc_set_fore_color(hdc, GUI_3D_DOWN_IN2_FCOLOR);
    in_line(hdc, trect.right-3, trect.top+1, trect.right-3, trect.bottom-2);
    in_line(hdc, trect.left+1, trect.bottom-3, trect.right-2, trect.bottom-3);

    in_hdc_set_current_group_color(hdc, FORE_ROLE, old_color);
    in_hdc_release_win(p, hdc);

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
 
    return  1;
}
    
static  int  in_common_children_callback_recursion(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND  p      = NULL;
    HWND  temp_p = NULL;


    if ( msg == NULL )
        return  -1;
    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    temp_p = p;
    while  ( temp_p != NULL )
    {
        ((GUI_MESSAGE *)msg)->to_hwnd = temp_p;
        in_callback_to_hwnd_message(msg);
        if ( (temp_p->head.fc) != NULL )
        {
            ((GUI_MESSAGE *)msg)->to_hwnd = temp_p->head.fc;
            in_common_children_callback_recursion(msg);
        }

        temp_p = temp_p->head.next;
    }

    return  1;
}

int  in_common_widget_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND  p = NULL;


    if ( msg == NULL )
        return  -1;
    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch ( MESSAGE_GET_ID(msg) )
    {
        case  MSG_CLOSE:
            in_win_close(p);
            return  0;

        case  MSG_MAXIZE:
            in_win_maxize(p);
            return  0;

        case  MSG_NORMAL:
            in_win_normal(p);
            return  0;

        case  MSG_HIDE:
            in_win_hide(p);
            return  0;

        case  MSG_PAINT:
            if ( (p->head.fc) != NULL )
            {
                ((GUI_MESSAGE *)msg)->to_hwnd = p->head.fc;
                in_common_children_callback_recursion(msg);
            }
            break;

        case  MSG_GET_FOCUS:
        case  MSG_LOST_FOCUS:
            in_win_invalidate(p);
            return  0;

        default:
            break;
    }

    return  1;
}

#endif  /* _LG_WINDOW_ */
