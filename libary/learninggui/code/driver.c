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

#include  <driver.h>
#include  <screen.h>
#include  <keyboard.h>
#include  <mtjt.h>
#include  <lock.h>


int  in_driver_register(unsigned int driver_type, void *driver)
{
    if ( driver_type == DRIVER_SCREEN )
    {
        lscrn  = *((GUI_SCREEN *)driver);      
        return  1;
    }

    #ifdef  _LG_KEYBOARD_ 
    if ( driver_type == DRIVER_KEYBOARD )
    {
        lkbd  = *((GUI_KEYBOARD *)driver);      
        return  1;
    }
    #endif  /* _LG_KEYBOARD_ */

    #ifdef   _LG_MTJT_
    if ( driver_type == DRIVER_MTJT )
    {
        lmtjt  = *((GUI_MTJT *)driver);      
        return  1;
    }
    #endif  /* _LG_MTJT_ */

    #ifdef  _LG_MULTI_THREAD_
    if ( driver_type == DRIVER_THREAD_GUI_LOCKER )
    {
        lglckr = *((GUI_LOCKER *)driver);      
        return  1;
    }

    if ( driver_type == DRIVER_THREAD_CALLBACK_LOCKER )
    {
        lclckr = *((GUI_LOCKER *)driver);      
        return  1;
    }
    #endif  /* _LG_MULTI_THREAD_ */

    return  0;
}
