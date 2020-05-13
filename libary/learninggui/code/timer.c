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

#include  <stdio.h>
#include  <string.h>
/* ?? */
#include  <time.h>

#include  <lock.h>

#include  <timer.h>

 
#ifdef  _LG_TIMER_
static  volatile  unsigned int  ltmnum   = 0;
static  volatile  IN_GUI_TIMER  ltmque[MAX_TIMER] = {{0}};

    
int  in_gui_timer_init(void)
{
    memset((void *)ltmque, 0, sizeof(ltmque));
    ltmnum = 0;

    return  1;
}

int  in_gui_timer_create_ext(unsigned int id, unsigned int microsecond, void *para)
{
    IN_GUI_TIMER      *p;
    struct  timespec  tp;
    unsigned int      i = 0;


    if ( ltmnum >= MAX_TIMER )
        return  -1;


    for ( i = 0; i < MAX_TIMER; i++ )
    {
        p = (IN_GUI_TIMER *)(&ltmque[i]);
        if ( (p->flag) != 0 )
            continue;


        clock_gettime(CLOCK_MONOTONIC, &tp);

        p->flag           = 1;
        p->id             = id;
        p->microsecond    = microsecond;
        p->last_tp        = tp;
        p->para           = para;
       
        ltmnum++;
        if ( ltmnum > MAX_TIMER )
            ltmnum = MAX_TIMER;

        return  1;
    }

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  gui_timer_create_ext(unsigned int id, unsigned int microsecond, void *para)
{
    int  ret = 0;

    gui_lock();
    ret = in_gui_timer_create_ext(id, microsecond, para);
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_gui_timer_delete(unsigned int id)
{
    IN_GUI_TIMER     *p;
    unsigned int     i = 0;

    for ( i = 0; i < MAX_TIMER; i++ )
    {
        p = (IN_GUI_TIMER *)(&ltmque[i]);
        if ( (p->id) != id )
            continue;

        p->flag         = 0;

        ltmnum--;
        if ( ltmnum > MAX_TIMER )
            ltmnum = 0;

        return  1;

    }

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  gui_timer_delete(unsigned int id)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_timer_delete(id);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_gui_timer_get_expired(void *gui_timer)
{
    IN_GUI_TIMER      *p;
    struct  timespec  tp;
    unsigned int      micro_delta = 0;    
    unsigned int      num         = 0;
    unsigned int      i           = 0;


    if ( gui_timer == 0 )
        return  -1;


    if ( ltmnum == 0 )
        return  0;

    
    clock_gettime(CLOCK_MONOTONIC, &tp);

    num = 0;
    for ( i = 0; i < MAX_TIMER; i++ )
    {
        p = (IN_GUI_TIMER *)(&ltmque[i]);

        if ( (p->flag) ==  0 )
            continue;
 
        micro_delta = (tp.tv_sec - p->last_tp.tv_sec)*1000000 + (tp.tv_nsec - p->last_tp.tv_nsec)/1000;      
        if ( micro_delta < (p->microsecond-TIMER_OFFSET) )
            goto  NEXT_TIMER;

        p->last_tp    = tp;
        *((IN_GUI_TIMER *)gui_timer) = *p;
        return  1;

        NEXT_TIMER:
        num++;
        if ( num < ltmnum )
            continue;

        return  0;
    }

    return  0;

}
#endif  /* _LG_TIMER_ */
