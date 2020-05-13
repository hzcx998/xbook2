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

#ifndef  __LGUI_CURSOR_HEADER__
#define  __LGUI_CURSOR_HEADER__

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



/* Cursor type */
#define  CURSOR_NORMAL              0x00
#define  CURSOR_WAITING             0x01
#define  CURSOR_CAPTURED            0x02

#define  MAX_CURSORS                (CURSOR_CAPTURED+1)


#ifdef  _LG_CURSOR_    
    
#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal Function */
    int  in_cursor_init(void);

    int  in_cursor_enable(void);
    int  in_cursor_disable(void);

    int  in_cursor_show(void);
    int  in_cursor_hide(void);

    int  in_cursor_get_position(void *point);
    int  in_cursor_set_position(int x, int y);

    int  in_cusor_set_shape(unsigned int cursor_id, const void *shape);

    int  in_cursor_get_id(void);
    int  in_cursor_set_id(unsigned int cursor_id);

    /* ?? */
    int  in_cursor_maybe_restore_back_abs(int left, int top, int right, int bottom);
    int  in_cursor_maybe_refresh(void);


    #ifndef  _LG_ALONE_VERSION_
    int  cursor_enable(void);
    int  cursor_disable(void);

    int  cursor_show(void);
    int  cursor_hide(void);

    int  cursor_get_position(void *point);
    int  cursor_set_position(int x, int y);

    int  cusor_set_shape(unsigned int cursor_id, const void *shape);

    int  cursor_get_id(void);
    int  cursor_set_id(unsigned int cursor_id);

    #ifdef  _LG_WINDOW_
    int  cursor_maybe_restore_back_abs(int left, int top, int right, int bottom);
    int  cursor_maybe_refresh(void);
    #endif

    #else  /* _LG_ALONE_VERSION_ */
    #define  cursor_enable()                                          in_cursor_enable()
    #define  cursor_disable()                                         in_cursor_disable()

    #define  cursor_show()                                            in_cursor_show()
    #define  cursor_hide()                                            in_cursor_hide()

    #define  cursor_get_position(point)                               in_cursor_get_position(point)
    #define  cursor_set_position(x, y)                                in_cursor_set_position(x, y)

    #define  cursor_set_shape(cursor_id, shape)                       in_cursor_set_shape(cursor_id, shape)

    #define  cursor_get_id()                                          in_cursor_get_id()
    #define  cursor_set_id(cursor_id)                                 in_cusrod_set_id(cursor_id)

    #ifdef  _LG_WINDOW_
    #define  cursor_maybe_restore_back_abs(left, top, right, bottom)  in_cursor_maybe_restore_back_abs(left, top, right, bottom)
    #define  cursor_maybe_refresh()                                   in_cursor_maybe_refresh()
    #endif

    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif
   
#endif  /* _LG_CURSOR_ */ 
 
#endif  /* __LGUI_CURSOR_HEADER__ */
