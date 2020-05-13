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

#include  <win_cell.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"


#ifdef  _LG_WINDOW_
#ifdef  _LG_CELL_WIDGET_

/* Cell widget internal callback */
static  int  in_cell_callback(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;

 
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

            in_win_message_send_ext(p, MSG_PAINT_NEXT, HWND_APP_CALLBACK);            
            break;

        default:
            break;
    }

    return  in_common_widget_callback(msg);
}

HWND in_cell_create(HWND parent, void *gui_common_widget, void *gui_cell)
{
    HWND    p = NULL;
    unsigned int size = 0;
    int     ret;


    /* Adjust parent */
    if ( parent == NULL )
        parent = HWND_DESKTOP;
    if ( in_win_has(parent) < 0 )
        return  NULL;


    /* 
     * set hwnd common 
     */
    /* ID, style, ext_style, default win_dc */
    /* Border ?? */
    /* set ltcomm and ltdc value */
    ret = in_set_hwnd_common1(parent, CELL_WIDGET_TYPE, gui_common_widget);
    if ( ret < 1 )
        return  NULL;


    /* set hwnd common win_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE] = CELL_WIN_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE] = CELL_WIN_DISABLED_FCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE] = CELL_WIN_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE] = CELL_WIN_INACTIVE_FCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]   = CELL_WIN_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]   = CELL_WIN_ACTIVE_FCOLOR;
 
    ltcomm.win_dc = ltdc; 


    /* set hwnd common2 */
    /* set client dc */
    in_set_hwnd_common2(gui_common_widget);

    /* set hwnd common client_dc */
    ltdc.color[DISABLED_GROUP][BACK_ROLE]      = CELL_CLI_DISABLED_BCOLOR;
    ltdc.color[DISABLED_GROUP][FORE_ROLE]      = CELL_CLI_DISABLED_FCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_BACK_ROLE] = CELL_CLI_DISABLED_TBCOLOR;
    ltdc.color[DISABLED_GROUP][TEXT_FORE_ROLE] = CELL_CLI_DISABLED_TFCOLOR;

    ltdc.color[INACTIVE_GROUP][BACK_ROLE]      = CELL_CLI_INACTIVE_BCOLOR;
    ltdc.color[INACTIVE_GROUP][FORE_ROLE]      = CELL_CLI_INACTIVE_FCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_BACK_ROLE] = CELL_CLI_INACTIVE_TBCOLOR;
    ltdc.color[INACTIVE_GROUP][TEXT_FORE_ROLE] = CELL_CLI_INACTIVE_TFCOLOR;

    ltdc.color[ACTIVE_GROUP][BACK_ROLE]        = CELL_CLI_ACTIVE_BCOLOR;
    ltdc.color[ACTIVE_GROUP][FORE_ROLE]        = CELL_CLI_ACTIVE_FCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = CELL_CLI_ACTIVE_TBCOLOR;
    ltdc.color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = CELL_CLI_ACTIVE_TFCOLOR;

    ltcomm.client_dc = ltdc; 


    /* set hwnd common3 */
    /* ltcomm */
    in_set_hwnd_common3(gui_common_widget);
    ltcomm.in_callback = in_cell_callback;
 

    /* malloc */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_CELL);
    p = in_malloc_hwnd_memory(gui_common_widget, size);
    if (p == NULL)
        return  NULL;


    /* Ext */


    /* 
     * add hwnd and post message.
     */
    ret = in_deal_add_hwnd(gui_common_widget, p, 0);
    if ( ret < 1)
        return  NULL;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  cell_create(HWND parent, void *gui_common_widget, void *gui_cell)
{
    HWND  p = NULL;

    gui_lock( );
    p = in_cell_create(parent, gui_common_widget, gui_cell);
    gui_unlock( );

    return  p;
}
#endif

#endif  /* _LG_CELL_WIDGET_ */
#endif  /* _LG_WINDOW_ */
