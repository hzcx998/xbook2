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

#include  <screen.h>
#include  <mtjt.h>


#ifdef  _LG_MTJT_
volatile  GUI_MTJT  lmtjt;

    
int  in_mtjt_open(void)
{
    int  ret = 0;


    if ((lmtjt.open) == NULL)
        return  -1;

    ret = (lmtjt.open)();

    #ifdef  _LG_SCREEN_
    lmtjt.cur_x = (lscrn.width)/2;
    lmtjt.cur_y = (lscrn.height)/2;
    #endif

    #ifdef  _LG_CURSOR_
    in_cursor_set_position((lmtjt.cur_x), (lmtjt.cur_y));
    #endif

    return  ret;
}

int  in_mtjt_close(void)
{
    if ((lmtjt.close) == NULL)
        return  -1;

    return  (lmtjt.close)();
}


int  in_mtjt_get_point_abs(GUI_POINT *point)
{
    if ( point == NULL )
        return  -1;

    point->x = (lmtjt.cur_x);
    point->y = (lmtjt.cur_y);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  mtjt_get_point_abs(GUI_POINT *point)
{
    int  ret = 0;

    if ( point == NULL )
        return  -1;
 
    gui_lock();
    ret = in_mtjt_get_point_abs(point);
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_mtjt_set_point_abs(GUI_POINT *point)
{
    if ( point == NULL )
        return  -1;

    (lmtjt.cur_x) = point->x;
    (lmtjt.cur_y) = point->y;

    if ((lmtjt.cur_x) < 0)
        (lmtjt.cur_x) = 0;
    if ((lmtjt.cur_x) > ((lscrn.width)-1))
        (lmtjt.cur_x) = (lscrn.width)-1;
 
    if ((lmtjt.cur_y) < 0)
        (lmtjt.cur_y) = 0;
    if ((lmtjt.cur_y) > ((lscrn.height)-1))
        (lmtjt.cur_y) = (lscrn.height)-1;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  mtjt_set_point_abs(GUI_POINT *point)
{
    int  ret = 0;

    if ( point == NULL )
        return  -1;
 
    gui_lock();
    ret = in_mtjt_set_point_abs(point);
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_mtjt_set_point_rel(GUI_POINT *point)
{
    if ( point == NULL )
        return  -1;
   
    (lmtjt.cur_x) += point->x;
    (lmtjt.cur_y) += point->y;

    if ((lmtjt.cur_x) < 0)
        (lmtjt.cur_x) = 0;

    if ((lmtjt.cur_x) > ((lscrn.width)-1))
        (lmtjt.cur_x) = (lscrn.width)-1;

    if ((lmtjt.cur_y) < 0)
        (lmtjt.cur_y) = 0;

    if ((lmtjt.cur_y) > ((lscrn.height)-1))
        (lmtjt.cur_y) = (lscrn.height)-1;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  mtjt_set_point_rel(GUI_POINT *point)
{
    int  ret = 0;

    if ( point == NULL )
        return  -1;

    gui_lock();
    ret = in_mtjt_set_point_rel(point);
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_mtjt_control(void *p1, void *p2)
{
    int  ret = 0;

    ret = (lmtjt.control)(p1, p2);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  mtjt_control(void *p1, void *p2)
{
    int  ret = 0;

    gui_lock( );
    ret = in_mtjt_control(p1, p2); 
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_mtjt_reinit(void)
{
    int  ret = 0;

    ret = (lmtjt.reinit)();

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  mtjt_reinit(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_mtjt_reinit(); 
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_MTJT_ */
