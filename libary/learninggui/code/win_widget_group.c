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

#include  <win_type_widget.h>
#include  <win_widget.h>
#include  <win_desktop.h>
#include  <win_interface.h>
#include  <win_radio_button.h> 
#include  <win_widget_group.h> 

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"

    
#ifdef  _LG_WINDOW_
#ifdef  _LG_WIDGET_GROUP_

static  volatile  HWND  lhwgrp = NULL;


int   in_widget_group_init(void)
{
    lhwgrp = NULL;

    return  1;
}
    
int   in_widget_group_delete_by_parent(/* HWND hwnd */ void *parent)
{
    HWND  p        = NULL;
    HWND  tmp_prev = NULL;
    HWND  tmp_next = NULL;


    if ( lhwgrp == NULL )
        return  0;
    if ( parent == NULL )
        return  0;


    p = lhwgrp;
    while ( p != NULL )
    {
        if ( (p->head.parent) == (HWND)parent )
        {
            tmp_prev = p->head.prev;
            tmp_next = p->head.next;
            if ( tmp_prev != NULL )
                tmp_prev->head.next = tmp_next;
            if ( tmp_next != NULL )
               tmp_next->head.prev = tmp_prev;


           if ( p == lhwgrp )
               lhwgrp = tmp_prev;


            free(p);

            p = tmp_prev;
            continue;
        }

        p = p->head.prev;

    }

    return  0;
}

/* WidgetGroup internal callback */
static  int  in_widget_group_callback(/* GUI_MESSAGE *msg */ void *msg)
{

    return  0;
}

HWND in_widget_group_create(HWND parent)
{
    HWND                      p      = NULL;
    IN_GUI_WIDGET_GROUP      *pgroup = NULL;
    unsigned int              size   = 0;


    if ( parent == NULL )
        return  NULL;


    /* Head */
    memset((void *)(&lthead), 0, sizeof(GUI_WND_HEAD));
    lthead.prev    = NULL;
    lthead.next    = NULL;
    lthead.parent  = parent;
    lthead.fc      = NULL;
    lthead.lc      = NULL;

    memset((void *)(&ltcomm), 0, sizeof(GUI_COMMON_WIDGET));


    /* Set id */
    ltcomm.id                    = -1;

    /* Set type */
    ltcomm.type                  = WIDGET_GROUP_TYPE;

    /* Style and ext style */
    ltcomm.style                 = ~VISUAL_STYLE; 
    ltcomm.ext_style             = 0;


    #ifdef  _LG_WIDGET_USER_DATA_
    /* User data max bytes */
    ltcomm.max_data_bytes        = MAX_USER_DATA_LEN; 

    /* User data bytes */
    ltcomm.data_bytes            = 0; 
    #endif

    /* Callback function */
    ltcomm.no_focus_flag         = 1;
    ltcomm.is_in_callback        = 1;
    ltcomm.in_callback           = in_widget_group_callback;


    /* 
     * malloc memory.
     */
    size = sizeof(GUI_WND_HEAD)+sizeof(GUI_COMMON_WIDGET)+sizeof(IN_GUI_WIDGET_GROUP);

    /* User data */
    #ifdef  _LG_WIDGET_USER_DATA_
    size += ltcomm.max_data_bytes; 
    #endif

    p = (HWND)malloc(size); 
    if ( p == NULL )
        return  NULL;
    memset(p, 0, size);

    /* Copy data from user side to memory */
    memcpy(&(p->head), (const void *)(&lthead), sizeof(GUI_WND_HEAD));
    memcpy(&(p->common), (const void *)(&ltcomm), sizeof(GUI_COMMON_WIDGET));

    p->common.win_dc.hwnd     = p;
    p->common.client_dc.hwnd  = p;
    p->common.invalidate_flag = 0;

    pgroup = (IN_GUI_WIDGET_GROUP *)(p->ext);
    pgroup->checked_hwnd = NULL;
    pgroup->checked_flag = 0;


    if (lhwgrp == NULL)
    {
        lhwgrp = p;
        return  p;
    }

    p->head.prev = lhwgrp;
    lhwgrp->head.next = p;

    lhwgrp = p;

    return  p;
}

#ifndef  _LG_ALONE_VERSION_
HWND  widget_group_create(HWND parent)
{
    HWND  hwnd = NULL;

    gui_lock( );
    hwnd = in_widget_group_create(parent);
    gui_unlock( );

    return  hwnd;
}
#endif

int  in_widget_group_close(HWND widget_group)
{
    HWND  pgrp     = widget_group;

    HWND  p        = NULL;
    HWND  tmp_prev = NULL;
    HWND  tmp_next = NULL;


    if ( pgrp == NULL )
        return  -1;
    if ( (pgrp->common.type) != WIDGET_GROUP_TYPE )
        return  -1;


    p = lhwgrp;
    while ( p != NULL )
    {
        if ( p == pgrp )
        {
            tmp_prev = p->head.prev;
            tmp_next = p->head.next;
            if ( tmp_prev != NULL )
                tmp_prev->head.next = tmp_next;
            if ( tmp_next != NULL )
               tmp_next->head.prev  = tmp_prev;


           if ( p == lhwgrp )
               lhwgrp = tmp_prev;


            free(p);
            p = NULL;

            return  1;
        }
        p = p->head.prev;
    }

    return  0;
}

#ifndef  _LG_ALONE_VERSION_
int  widget_group_close(HWND widget_group)
{
    int  ret = 0;

    gui_lock( );
    ret = in_widget_group_close(widget_group);
    gui_unlock( );

    return  ret;
}
#endif


int  in_attach_widget(HWND hwnd, HWND widget_group)
{
    HWND                       p = hwnd;
    IN_GUI_RADIO_BUTTON       *pin_radio = NULL;

 
    if ( p == NULL )
        return  -1;
    if ( widget_group == NULL )
        return  -1;


    pin_radio = (IN_GUI_RADIO_BUTTON *)(p->ext);
    if ( pin_radio == NULL )
        return  -1;
    pin_radio->group_hwnd     = widget_group;
    pin_radio->connected_flag = 1;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  attach_widget(HWND hwnd, HWND widget_group)
{
    int  ret = 0;

    gui_lock( );
    ret = in_attach_widget(hwnd, widget_group);
    gui_unlock( );

    return  ret;
}
#endif
 
int   in_detach_widget(HWND hwnd)
{
    HWND                   p         = hwnd;
    IN_GUI_WIDGET_GROUP   *pin_group = NULL;
    IN_GUI_RADIO_BUTTON   *pin_radio = NULL;

 
    if ( p == NULL )
        return  -1;
    if ((p->common.type) != RADIO_BUTTON_WIDGET_TYPE)
        return  -1;


    pin_radio = (IN_GUI_RADIO_BUTTON *)(p->ext);
    if ( pin_radio == NULL )
        return  -1;

    if ((pin_radio->group_hwnd) == NULL )
        goto  DETACH_END;

    pin_group =(IN_GUI_WIDGET_GROUP *)(pin_radio->group_hwnd)->ext;
    if (pin_group == NULL )
        goto  DETACH_END;

    if ((pin_group->checked_hwnd) != p)
        goto  DETACH_END;

    pin_group->checked_hwnd = NULL;
    pin_group->checked_flag = 0;


    DETACH_END:
    pin_radio->group_hwnd     = NULL;
    pin_radio->connected_flag = 0;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int   detach_widget(HWND hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_detach_widget(hwnd);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_WIDGET_GROUP_ */
#endif  /* _LG_WINDOW_ */
