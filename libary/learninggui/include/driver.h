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

#ifndef  __LGUI_GUI_DRIVER_HEADER__
#define  __LGUI_GUI_DRIVER_HEADER__

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


#define  DRIVER_SCREEN                            0x01
#define  DRIVER_KEYBOARD                          0x02
#define  DRIVER_MTJT                              0x03
#define  DRIVER_THREAD_GUI_LOCKER                 0x04
#define  DRIVER_THREAD_CALLBACK_LOCKER            0x05


#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_driver_register(unsigned int driver_type, void *driver);

#ifdef  __cplusplus
}
#endif

#endif  /* __LGUI_GUI_DRIVER_HEADER__ */
