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

#include  <image_comm.h>
#include  <image_bitmap.h>
#include  <image_icon.h>
#include  <image_gif.h>

#include  <win_tools.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_widget.h>
#include  <win_image.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_IMAGE_WIDGET_

/* Image widget internal callback */
static  int  in_image_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND               p;
    HDC                hdc;
    GUI_RECT           rect;
    const  GUI_GIF    *pgif = NULL;
    int                left = 0;
    int                top  = 0;

 
    if ( msg == NULL )
        return  -1;

    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch ( MESSAGE_GET_ID(msg) )
    {
        /* default back color ? */
        case  MSG_PAINT:
            if ( in_win_is_visual(p) < 1 )
                break;

            in_win_set_current_color_group(p);

            if ( IS_BORDER_WIDGET(&(p->common)) > 0 )
            {
                if ( IS_BORDER_3D_WIDGET(&(p->common)) > 0 )
                    in_paint_3d_up_border(p, &(p->common.win_dc.rect));
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


            /* PAINT_IMAGE_BITMAP: */
            if ( (GET_IN_GUI_IMAGE(p)->image_type) != IMAGE_BITMAP )
                goto  PAINT_IMAGE_ICON;

            if ( (GET_IN_GUI_IMAGE(p)->image_align) != IMAGE_ALIGN_FILL )
                goto  PAINT_IMAGE_NO_FILL_BITMAP;

            in_bitmap_fill_rect(hdc, &rect, GET_IN_GUI_IMAGE(p)->pimage);
            goto  PAINT_IMAGE_OK;

            PAINT_IMAGE_NO_FILL_BITMAP:
            in_bitmap_fill(hdc, left, top, GET_IN_GUI_IMAGE(p)->pimage);
            goto  PAINT_IMAGE_OK;


            PAINT_IMAGE_ICON:
            if ( (GET_IN_GUI_IMAGE(p)->image_type) != IMAGE_ICON )
                goto  PAINT_IMAGE_GIF;

            if ( (GET_IN_GUI_IMAGE(p)->image_align) != IMAGE_ALIGN_FILL )
                goto  PAINT_IMAGE_NO_FILL_ICON;

            in_icon_fill_rect(hdc, &rect, GET_IN_GUI_IMAGE(p)->pimage);
            goto  PAINT_IMAGE_OK;

            PAINT_IMAGE_NO_FILL_ICON:
            in_icon_fill(hdc, left, top, GET_IN_GUI_IMAGE(p)->pimage);
            goto  PAINT_IMAGE_OK;


            PAINT_IMAGE_GIF:
            if ( (GET_IN_GUI_IMAGE(p)->image_type) != IMAGE_GIF )
                goto  PAINT_IMAGE_OK;

            pgif = GET_IN_GUI_IMAGE(p)->pimage;
            if ( GET_IN_GUI_IMAGE(p)->frame_id > (pgif->block_num - 1) )
                GET_IN_GUI_IMAGE(p)->frame_id = 0;
            in_gif_man_play(hdc, rect.left, rect.top, GET_IN_GUI_IMAGE(p)->pimage, GET_IN_GUI_IMAGE(p)->frame_id);
            GET_IN_GUI_IMAGE(p)->frame_id++;


            PAINT_IMAGE_OK:
            in_hdc_release_win(p, hdc);
            p->common.invalidate_flag = 0;

            #ifdef  _LG_CURSOR_
            in_cursor_maybe_refresh( );
            #endif
 
            in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_IN_CALLBACK | HWND_APP_CALLBACK); 
            return  0;

        case  MSG_COUNTER:
        case  MSG_TIMER:
            in_win_message_send_ext(p, MSG_PAINT, HWND_IN_CALLBACK | HWND_APP_CALLBACK);
            return  0;

        default:
            break;
    }

    return  in_common_widget_callback(msg);
}

HWND in_image_create(HWND parent, void *gui_common_widget, void *gui_image)
{
    HWND                p     = NULL;
    GUI_IMAGE          *image = (GUI_IMAGE *)gui_image;
    IN_GUI_IMAGE        in_image;
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
    ret = in_set_hwnd_common1(parent, IMAGE_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;


    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = IMAGE_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = IMAGE_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = IMAGE_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = IMAGE_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = IMAGE_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = IMAGE_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc                         = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = IMAGE_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = IMAGE_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = IMAGE_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = IMAGE_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = IMAGE_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = IMAGE_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = IMAGE_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = IMAGE_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = IMAGE_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = IMAGE_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = IMAGE_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = IMAGE_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc                    = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.no_focus_flag      = 1;
    ltcomm.no_erase_back_flag = 1;
    ltcomm.in_callback = in_image_callback;


    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_IMAGE);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */
    memset(&in_image, 0, sizeof(in_image));
    if ( image != NULL )
    {
        in_image.image_type    = image->image_type;
        in_image.image_align   = image->image_align;
        in_image.pimage        = image->pimage;
    }
    memcpy(p->ext, &in_image, sizeof(IN_GUI_IMAGE));


    /* 
     * add hwnd and post message.
     */
    ret = in_deal_add_hwnd(gui_common_widget, p, 0);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  image_create(HWND parent, void *gui_common_widget, void *gui_image)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_image_create(parent, gui_common_widget, gui_image);
    gui_unlock( );

    return  p;
}
#endif


int  in_image_set_image(HWND hwnd, void *gui_image)
{
    HWND   p         = hwnd;
    GUI_IMAGE *pdata = (GUI_IMAGE *)gui_image;


    if ( p == NULL )
        return  -1;
    if ( pdata == NULL )
        return  -1;
    if ( (p->common.type) != IMAGE_WIDGET_TYPE )
        return  -1;

    GET_IN_GUI_IMAGE(p)->image_type = pdata->image_type;
    GET_IN_GUI_IMAGE(p)->pimage     = pdata->pimage;

    in_win_message_post_ext(p, MSG_PAINT, HWND_APP_CALLBACK | HWND_IN_CALLBACK);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  image_set_image(HWND hwnd, void *gui_image)
{
    int  ret = 0;

    gui_lock( );
    ret = in_image_set_image(hwnd, gui_image);
    gui_unlock( );

    return  ret;
}
#endif

int  in_image_get_image(HWND hwnd, void *gui_image)
{
    HWND   p         = hwnd;
    GUI_IMAGE *pdata = (GUI_IMAGE *)gui_image;


    if ( p == NULL )
        return  -1;
    if ( pdata == NULL )
        return  -1;
    if ( (p->common.type) != IMAGE_WIDGET_TYPE )
        return  -1;

    pdata->image_type = GET_IN_GUI_IMAGE(p)->image_type;
    pdata->pimage     = GET_IN_GUI_IMAGE(p)->pimage;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  image_get_image(HWND hwnd, void *gui_image)
{
    int  ret = 0;

    gui_lock( );
    ret = in_image_get_image(hwnd, gui_image);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_IMAGE_WIDGET_ */
#endif  /* _LG_WINDOW_ */
