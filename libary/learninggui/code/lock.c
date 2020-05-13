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


#ifdef  _LG_MULTI_THREAD_
GUI_LOCKER      lglckr = { 0 };
GUI_LOCKER      lclckr = { 0 };


int  gui_locker_init(void)
{
    if ( (lglckr.init) == NULL )
        return  0;

    return  (lglckr.init)();
}

int  gui_locker_destroy(void)
{
    if ( (lglckr.destroy) == NULL )
        return  0;

    return  (lglckr.destroy)();
}

#ifdef  _LG_INLINE_
DEFAULT_INLINE  int  gui_lock(void)
#else
int  gui_lock(void)
#endif
{
    if ( (lglckr.lock) == NULL )
        return  0;

    return  (lglckr.lock)();
}

#ifdef  _LG_INLINE_
DEFAULT_INLINE  int  gui_unlock(void)
#else
int  gui_unlock(void)
#endif
{
    if ( (lglckr.unlock) == NULL )
        return  0;

    return  (lglckr.unlock)();
}


int  callback_locker_init(void)
{
    if ( (lclckr.init) == NULL )
        return  0;

    return  (lclckr.init)();
}

int  callback_locker_destroy(void)
{
    if ( (lclckr.destroy) == NULL )
        return  0;

    return  (lclckr.destroy)();
}

#ifdef  _LG_INLINE_
DEFAULT_INLINE  int  callback_lock(void)
#else
int  callback_lock(void)
#endif
{
    if ( (lclckr.lock) == NULL )
        return  0;

    return  (lclckr.lock)();
}

#ifdef  _LG_INLINE_
DEFAULT_INLINE  int  callback_unlock(void)
#else
int  callback_unlock(void)
#endif
{
    if ( (lclckr.unlock) == NULL )
        return  0;

    return  (lclckr.unlock)();
}
#endif  /* _LG_MULTI_THREAD_ */
