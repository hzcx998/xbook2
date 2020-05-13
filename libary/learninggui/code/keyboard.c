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

#include  <lock.h>
#include  <cursor.h>

#include  <keyboard.h>



#ifdef  _LG_KEYBOARD_
volatile  GUI_KEYBOARD  lkbd = {0};

int  in_keyboard_open(void)
{
    if ( (lkbd.open) == NULL )
        return  -1;

    return  (lkbd.open)();

}

int  in_keyboard_close(void)
{
    if ( (lkbd.close) == NULL )
       return  -1;

    return  (lkbd.close)();
}


int  in_keyboard_control(void *p1, void *p2)
{
    int  ret = 0;

    ret = (lkbd.control)(p1, p2);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  keyboard_control(void *p1, void *p2)
{
    int  ret = 0;

    gui_lock( );
    ret = in_keyboard_control(p1, p2); 
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_keyboard_reinit(void)
{
    int  ret = 0;

    ret = (lkbd.reinit)();

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  keyboard_reinit(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_keyboard_reinit(); 
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_KEYBOARD_ */
