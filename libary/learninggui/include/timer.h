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

#ifndef  __LGUI_GUI_TIMER_HEADER__
#define  __LGUI_GUI_TIMER_HEADER__

#include  <time.h>

#include  <type_color.h>

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif

#include  <type_gui.h>

 
#ifdef  _LG_TIMER_

struct  _IN_GUI_TIMER
{
    BUINT            flag;
    unsigned int     id;
    unsigned int     microsecond;
    struct timespec  last_tp;
             void   *para;
};
typedef	struct	_IN_GUI_TIMER  IN_GUI_TIMER;


#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal Function */
    int  in_gui_timer_init(void);
    int  in_gui_timer_get_expired(void *gui_timer);
    /* Internal Function end */


    int  in_gui_timer_create_ext(unsigned int id, unsigned int microsecond, void *para);
    int  in_gui_timer_delete(unsigned int id);
    #define  in_gui_timer_create(id, millisecond,para)    in_gui_timer_create_ext(id, (millisecond*1000),para)

    #ifndef  _LG_ALONE_VERSION_
    int  gui_timer_create_ext(unsigned int id, unsigned int microsecond, void *para);
    int  gui_timer_delete(unsigned int id);
    #define  gui_timer_create(id, millisecond,para)        gui_timer_create_ext(id, (millisecond*1000),para)
    #else
    #define  gui_timer_create_ext(id, microsecond, para)   in_gui_timer_create_ext(id, microsecond, para)
    #define  gui_timer_delete(id)                          in_gui_timer_delete(id)
    #define  gui_timer_create(id, millisecond,para)        in_gui_timer_create_ext(id, (millisecond*1000),para)
    #endif /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_TIMER_ */

#endif  /* __LGUI_GUI_TIMER_HEADER__ */
