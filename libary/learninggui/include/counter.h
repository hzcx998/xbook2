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

#ifndef  __LGUI_COUNTER_HEADER__
#define  __LGUI_COUNTER_HEADER__

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

   
#ifdef  _LG_COUNTER_

struct  _IN_GUI_COUNTER
{
    unsigned char   flag;
    unsigned int    id;
    unsigned int    counter;
    unsigned int    last_counter;
             void  *para;
};
typedef	struct	_IN_GUI_COUNTER  IN_GUI_COUNTER;

    
#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal Function */
    int  in_gui_counter_init(void);
    int  in_gui_counter_get_expired(void *gui_counter);
    /* Internal Function end */

    int  in_gui_counter_create(unsigned int id, unsigned int counter, void *para);
    int  in_gui_counter_delete(unsigned int id);

    #ifndef  _LG_ALONE_VERSION_
    int  gui_counter_create(unsigned int id, unsigned int counter, void *para);
    int  gui_counter_delete(unsigned int id);
    #else  /* _LG_ALONE_VERSION_ */
    #define  gui_counter_create(id, counter, para)     in_gui_counter_create(id, counter, para)
    #define  gui_counter_delete(id)                    in_gui_counter_delete(id)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_COUNTER_ */

#endif  /* __LGUI_COUNTER_HEADER__ */
