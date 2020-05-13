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

#include  <win_type_widget.h>

#include  <win_tools.h>
#include  <win_invalidate.h>

#include  <win_interface.h>
#include  <win_desktop.h>

#include  "win_widget_group_in.h"


#ifdef  _LG_WINDOW_

/*
 * Global Window managing variables 
 */

/* Global  hwnd list */
volatile  HWND  lhlist = NULL;

/* Global focus hwnd */
volatile  HWND  lhfocu = NULL;

/* Global default hwnd */
volatile  HWND  lhdefa = NULL;

/* Global MTJT hwnd */
#ifdef   _LG_MTJT_
volatile  HWND  lhmtjt = NULL;
#endif


/* Temp GUI_DC buffer */
volatile  GUI_DC              ltdc;

/* Temp head buffer */
volatile  GUI_WND_HEAD        lthead;

/* Temp common widget buffer */
volatile  GUI_COMMON_WIDGET   ltcomm;


/* lhlist init address */
static volatile  HWND  hlsadd = NULL;


   
int  in_win_arithmetic_init(void)
{
    hlsadd = (HWND)(&lhlist);

    return  1;
}


static  int  in_win_has_recursion(/* HWND hwnd*/ void *hwnd, /* HWND hwnd*/ void *list)
{
    HWND  p      = (HWND)hwnd;
    HWND  list_p = (HWND)list;
    int   ret    = 0;


    if ( p == NULL )
        return  -1;
    if ( list_p == NULL )
        return  -1;


    while ( list_p != NULL )
    {
        if ( list_p == p )
            return  1;

        list_p = list_p->head.next;
    }

    list_p = (HWND)list;
    while ( list_p != NULL )
    {
        if ( (list_p->head.fc) != NULL )
        {
            ret = in_win_has_recursion(p, list_p->head.fc);
            if ( ret > 0 )
                return  1;
        }

        list_p = list_p->head.next;
    }

    return  -1;
}

int  in_win_has(/* HWND hwnd*/ void *hwnd)
{
    HWND  p       = (HWND)hwnd;
    HWND  main_p  = NULL;
    int   ret     = 0;


    if ( p == NULL )
        return  -1;

    /* Main list */
    main_p = HWND_DESKTOP;
    while ( main_p != NULL )
    {
        if ( main_p == p )
            return  1;

        main_p = main_p->head.next;
    }

    /* Children list */
    main_p = HWND_DESKTOP;
    while ( main_p != NULL )
    {
        if ( (main_p->head.fc) != NULL )
        {
            ret = in_win_has_recursion(p, main_p->head.fc);
            if ( ret > 0 )
                return  1;
        }
        main_p = main_p->head.next;
    }

    return  -1;
}

/* Add hwnd from  user create_xxx API */
int  in_win_add(/* HWND hwnd */ void *hwnd)
{
    HWND  p        = (HWND)hwnd;
    HWND  parent_p = NULL;
    HWND  temp_p   = NULL;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;


    parent_p = p->head.parent;
    if ( parent_p == NULL )
        parent_p = HWND_DESKTOP;
  
  
    /* Main window */
    if ( parent_p == HWND_DESKTOP )
    {
        if ( lhlist == hlsadd ) 
            p->head.prev = NULL;
        else
            p->head.prev = lhlist;

        p->head.next = NULL;
        p->head.fc   = NULL;
        p->head.lc   = NULL;

        lhlist->head.next = p;

        lhlist = p;

        lhfocu = lhlist;
        lhdefa = lhlist;
        #ifdef   _LG_MTJT_
        lhmtjt = lhlist;
        #endif

        return  1;
    }

    /* Children windows */
    if ( (parent_p->head.fc) == NULL )
    {
        parent_p->head.fc = p;
        parent_p->head.lc = p;

        p->head.prev      = NULL;
        p->head.next      = NULL;
        p->head.fc        = NULL;
        p->head.lc        = NULL;
    } else {
        temp_p = parent_p->head.lc;

        p->head.prev      = temp_p;
        p->head.next      = NULL;
        p->head.fc        = NULL;
        p->head.lc        = NULL;

        temp_p->head.next = p;

        parent_p->head.lc = p;
    }

    return  1;
}

static  int  in_win_delete_child_recursion(/* HWND hwnd */ void *hwnd)
{
    HWND  p     = (HWND)hwnd;
    HWND  tmp_p = NULL;
    HWND  del_p = NULL;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;

    tmp_p = p->head.fc;
    while ( tmp_p != NULL ) 
    { 
        if ( (tmp_p->head.fc) != NULL )
            in_win_delete_child_recursion(p->head.fc);

        /* GroupBox ?? */
        in_make_callback_message(tmp_p, MSG_CLOSE_NEXT, HWND_APP_CALLBACK);
        if ( (tmp_p->common.delete_callback) > 0 )
            (tmp_p->common.delete_callback)(tmp_p);

        /* 
         * Default hwnd ? 
         */
        if (tmp_p == lhfocu)
            lhfocu = HWND_DESKTOP;

        #ifdef   _LG_MTJT_
        if (tmp_p == lhmtjt)
            lhmtjt = NULL;
        #endif
  
        del_p = tmp_p; 
        tmp_p = tmp_p->head.next;

        #ifdef  _LG_WIDGET_GROUP_
        in_widget_group_delete_by_parent(del_p);
        #endif

        free(del_p);
        del_p = NULL;
    }

    return  1;
}

int  in_win_delete_hide_comm(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;


    if ( (p == lhfocu)&&((p->head.parent) == HWND_DESKTOP) )
    {
        if ( lhlist != NULL )
        {
            lhfocu = lhlist;
            lhdefa = lhlist;
            #ifdef   _LG_MTJT_
            lhmtjt = lhlist;
            #endif

        } else {
            lhfocu = HWND_DESKTOP;
            lhdefa = HWND_DESKTOP;
            #ifdef   _LG_MTJT_
            lhmtjt = HWND_DESKTOP;
            #endif
        }

        return  1;

    }

    if ( (p == lhdefa)&&((p->head.parent) == HWND_DESKTOP) )
    {
        if ( lhlist != NULL )
        {
            lhfocu = lhlist;
            lhdefa = lhlist;
            #ifdef   _LG_MTJT_
            lhmtjt = lhlist;
            #endif

        } else {
            lhfocu = HWND_DESKTOP;
            lhdefa = HWND_DESKTOP;
            #ifdef   _LG_MTJT_
            lhmtjt = HWND_DESKTOP;
            #endif
        }

        return  1;

    }

    #ifdef   _LG_MTJT_
    if ( (p == lhmtjt)&&((p->head.parent) == HWND_DESKTOP) )
    {
        if ( lhlist != NULL )
        {
            lhfocu = lhlist;
            lhdefa = lhlist;
            lhmtjt = lhlist;

        } else {
            lhfocu = HWND_DESKTOP;
            lhdefa = HWND_DESKTOP;
            lhmtjt = HWND_DESKTOP;
        }

        return  1;
    }
    #endif

    return  1;
} 

int  in_win_delete(/* HWND hwnd */ void *hwnd)
{
    HWND  p = (HWND)hwnd;

    HWND  prev_p   = NULL;
    HWND  next_p   = NULL;
    HWND  parent_p = NULL;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;
    if ( (p->common.acc_hwnd_flag) > 0 )
        return  -1;


    /* Dispatch all message */
    in_message_dispatch_all();

    /* Invalidate area */
    in_win_invalidate_below_area_abs(p, &(p->common.win_dc.rect));

    /* Delete all children windows */
    in_win_delete_child_recursion(p);

    /* Delete itself */
    in_make_callback_message(p, MSG_CLOSE_NEXT, HWND_APP_CALLBACK);

    prev_p = p->head.prev;
    next_p = p->head.next;
    if ( prev_p != NULL ) 
        prev_p->head.next = next_p;    
    if ( next_p != NULL ) 
        next_p->head.prev = prev_p;

    if ( lhlist == p )
        lhlist = prev_p;

    /* Update fc and lc field ? */
    parent_p = p->head.parent;
    if ( parent_p == NULL )
        goto  DELETE_WINDOW_LAST;

    if ( (parent_p->head.fc) == p )
        parent_p->head.fc = next_p;

    if ( (parent_p->head.lc) == p )
        parent_p->head.lc = prev_p;


    DELETE_WINDOW_LAST:
    if ( (p->common.delete_callback) > 0 )
        (p->common.delete_callback)(p);

    in_win_delete_hide_comm(p);

    #ifdef  _LG_WIDGET_GROUP_
    in_widget_group_delete_by_parent(p);
    #endif

    free(p);
    p = NULL;

    return  1;
} 
#endif  /* _LG_WINDOW_ */
