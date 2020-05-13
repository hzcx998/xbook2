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

#ifndef  __LGUI_MTJT_HEADER__
#define  __LGUI_MTJT_HEADER__

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



struct  _GUI_MTJT
{
    int   cur_x;
    int   cur_y;

    int  (*open)(void);
    int  (*close)(void);

    int  (*read)(void *msg);
    int  (*write)(void *buffer, unsigned int len);

    int  (*control)(void *p1, void *p2);
    int	 (*reinit)(void);
};
typedef	struct	_GUI_MTJT  GUI_MTJT;


#ifdef  _LG_MTJT_

#ifdef  __cplusplus
extern  "C"
{
#endif

    extern  volatile  GUI_MTJT  lmtjt;

    /* Internal Function */
    int  in_mtjt_open(void);
    int  in_mtjt_close(void);
    /* Internal Function end */

    int  in_mtjt_get_point_abs(GUI_POINT *point);
    int  in_mtjt_set_point_abs(GUI_POINT *point);
    int  in_mtjt_set_point_rel(GUI_POINT *point);

    int  in_mtjt_control(void *p1, void *p2);
    int  in_mtjt_reinit(void);

    #ifndef  _LG_ALONE_VERSION_
    int  mtjt_get_point_abs(GUI_POINT *point);
    int  mtjt_set_point_abs(GUI_POINT *point);
    int  mtjt_set_point_rel(GUI_POINT *point);

    int  mtjt_control(void *p1, void *p2);
    int  mtjt_reinit(void);
    #else  /* _LG_ALONE_VERSION_ */
    #define  mtjt_get_point_abs(point)            in_mtjt_get_point_abs(point)
    #define  mtjt_set_point_abs(point)            in_mtjt_set_point_abs(point)
    #define  mtjt_set_point_rel(point)            in_mtjt_set_point_rel(point)

    #define  mtjt_control(p1, p2)                 in_mtjt_control(p1, p2)
    #define  mtjt_reinit()                        in_mtjt_reinit() 
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_MTJT_ */

#endif  /* __LGUI_MTJT_HEADER__ */
