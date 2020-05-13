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

#ifndef  __LGUI_LOCK_HEADER__
#define  __LGUI_LOCK_HEADER__

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif


#ifdef  _LG_MULTI_THREAD_

#define  UNLOCK_FLAG                       0x00
#define  LOCK_FLAG                         0x01


struct  _GUI_LOCKER
{
    int  (*init)(void);
    int  (*destroy)(void);
    int  (*lock)(void);
    int  (*unlock)(void);
};
typedef  struct  _GUI_LOCKER   GUI_LOCKER;



#ifdef  __cplusplus
extern  "C"
{
#endif

    extern  GUI_LOCKER    lglckr;
    extern  GUI_LOCKER    lclckr;

    int  gui_locker_init(void);
    int  gui_locker_destroy(void);

    #ifdef  _LG_INLINE_
    DEFAULT_INLINE  int  gui_lock(void);
    #else
    int  gui_lock(void);
    #endif

    #ifdef  _LG_INLINE_
    DEFAULT_INLINE  int  gui_unlock(void);
    #else
    int  gui_unlock(void);
    #endif


    int  callback_locker_init(void);
    int  callback_locker_destroy(void);

    #ifdef  _LG_INLINE_
    DEFAULT_INLINE  int  callback_lock(void);
    #else
    int  callback_lock(void);
    #endif

    #ifdef  _LG_INLINE_
    DEFAULT_INLINE  int  callback_unlock(void);
    #else
    int  callback_unlock(void);
    #endif

#ifdef  __cplusplus
}
#endif

#else  /* _LG_MULTI_THREAD_ */

    #define  gui_locker_init()
    #define  gui_locker_destroy()
    #define  gui_lock()
    #define  gui_unlock()

    #define  callback_locker_init()
    #define  callback_locker_destroy()
    #define  callback_lock()
    #define  callback_unlock()

#endif  /* _LG_MULTI_THREAD_ */

#endif  /*__LGUI_LOCK_HEADER__*/
