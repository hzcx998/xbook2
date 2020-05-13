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

#include  <lock.h>

#include  <screen.h>


#ifdef  _LG_SCREEN_

volatile  GUI_SCREEN    lscrn = {0};



int  in_screen_get_width_height(unsigned int *width, unsigned int *height)
{
    if ( width  == NULL )
        return  -1;
    if ( height == NULL )
        return  -1;

    *width  = (lscrn.width); 
    *height = (lscrn.height);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  screen_get_width_height(unsigned int *width, unsigned int *height)
{
    int  ret = 0;

    gui_lock( );
    ret = in_screen_get_width_height(width, height);
    gui_unlock( );

    return  ret;
}
#endif

int  in_screen_open(void)
{
    if ( (lscrn.open) == NULL )
        return  -1;

    return  (lscrn.open)();

}

int  in_screen_close(void)
{
    if ( (lscrn.close) == NULL )
        return  -1;

    return  (lscrn.close)();
}


int  in_screen_control(void *p1, void *p2)
{
    int  ret = 0;

    ret = (lscrn.control)(p1, p2);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  screen_control(void *p1, void *p2)
{
    int  ret = 0;

    gui_lock( );
    ret = in_screen_control(p1, p2); 
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_screen_on(void)
{
    int  ret = 0;

    ret = (lscrn.on)();

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  screen_on(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_screen_on(); 
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_screen_off(void)
{
    int  ret = 0;

    ret = (lscrn.off)();

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  screen_off(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_screen_off(); 
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_screen_reinit(void)
{
    int  ret = 0;

    ret = (lscrn.reinit)();

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  screen_reinit(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_screen_reinit(); 
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_SCREEN_ */
