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

#include  <lock.h>

#include  <counter.h>


#ifdef  _LG_COUNTER_

/* sizeof(int) == 8 ? */
#define  MAX_UINT_VALUE    ((sizeof(int)==2)?65536:4294967296)

static  volatile  unsigned int    lcnval  = 0;

static  volatile  unsigned int    lcnnum  = 0;
static  volatile  IN_GUI_COUNTER  lcnque[MAX_COUNTER] = { {0} };


int  in_gui_counter_init(void)
{
    memset((void *)lcnque, 0, sizeof(lcnque));
    lcnval = 0;
    lcnnum = 0;

    return  1;
}

int  in_gui_counter_create(unsigned int id, unsigned int counter, void *para)
{
    IN_GUI_COUNTER   *p = NULL;
    unsigned int      i = 0;


    if ( lcnnum >= MAX_COUNTER )
        return  -1;

    for ( i = 0; i < MAX_COUNTER; i++ )
    {
        p = (IN_GUI_COUNTER *)(&lcnque[i]);
        if ( (p->flag) != 0 )
            continue;

        p->flag         = 1;
        p->id           = id;
        p->counter      = counter;
        p->last_counter = lcnval;
        p->para         = para;

        lcnnum++;
        if ( lcnnum > MAX_COUNTER )
            lcnnum = MAX_COUNTER;

        return  1;
    }

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  gui_counter_create(unsigned int id, unsigned int counter, void *para)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_counter_create(id, counter, para);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_gui_counter_delete(unsigned int id)
{
    IN_GUI_COUNTER   *p = NULL;
    unsigned int      i = 0;

    for ( i = 0; i < MAX_COUNTER; i++ )
    {
        p = (IN_GUI_COUNTER *)(&lcnque[i]);
        if ( (p->id) != id )
            continue;

        p->flag  = 0;

        lcnnum--;
        if ( lcnnum > MAX_COUNTER )
            lcnnum = 0;

        return  1;

    }

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  gui_counter_delete(unsigned int id)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_counter_delete(id);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_gui_counter_get_expired(void *gui_counter)
{
    IN_GUI_COUNTER  *p     = NULL;
    unsigned int     delta = 0;
    unsigned int     num   = 0;
    unsigned int     i     = 0;


    lcnval++;

    if ( gui_counter == 0 )
        return  -1;

    num  = 0;
    for ( i = 0; i < MAX_COUNTER; i++ )
    {
        p = (IN_GUI_COUNTER *)(&lcnque[i]);
        if ( (p->flag) ==  0 )
            continue;

        delta =  ( MAX_UINT_VALUE + lcnval - (p->last_counter))%MAX_UINT_VALUE;
        if (delta >= (p->counter))
        {
            p->last_counter = lcnval;
            *((IN_GUI_COUNTER *)gui_counter) = *p;
            return  1;
        }

        num++;
        if ( num < lcnnum )
            continue;

        return  0;
    }

    return  0;

}
#endif  /* _LG_COUNTER_ */
