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

#ifndef  __LGUI_RECT_OPS_HEADER__
#define  __LGUI_RECT_OPS_HEADER__

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif

#include  <type_gui.h>


#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_merge_rect(GUI_RECT *pdest, const GUI_RECT *pr0, const GUI_RECT *pr1);
    int  in_intersect_rect(GUI_RECT *pdest, const GUI_RECT *pr0, const GUI_RECT *pr1); 
    int  in_reduce_rect(GUI_RECT *pdest, const GUI_RECT *pr0, int dist);
    int  in_move_delta_rect(GUI_RECT *pr0, int dx, int dy);

    int  in_is_intersect_rect(const GUI_RECT *pr0, const GUI_RECT *pr1);
    int  in_is_zero_rect(const GUI_RECT *pr);
    int  in_is_none_zero_rect(const GUI_RECT *pr);

    #ifndef  _LG_ALONE_VERSION_
    int  merge_rect(GUI_RECT *pdest, const GUI_RECT *pr0, const GUI_RECT *pr1);
    int  intersect_rect(GUI_RECT *pdest, const GUI_RECT *pr0, const GUI_RECT *pr1);
    int  reduce_rect(GUI_RECT *pdest, const GUI_RECT *pr0, int dist);
    int  move_delta_rect(GUI_RECT *pr0, int dx, int dy);

    int  is_intersect_rect(const GUI_RECT *pr0, const GUI_RECT *pr1);
    int  is_zero_rect(const GUI_RECT *pr);
    int  is_none_zero_rect(const GUI_RECT *pr);
    #else
    #define  merge_rect(pdest, pr0, pr1)                in_merge_rect(pdest, pr0, pr1)
    #define  intersect_rect(pdest, pr0, pr1)            in_intersect_rect(pdest, pr0, pr1) 
    #define  reduce_rect(pdest, pr0, dist)              in_reduce_rect(pdest, pr0, dist)  
    #define  move_delta_rect(pr0, dx, dy)               in_move_delta_rect(pr0, dx, dy) 

    #define  is_intersect_rect(pr0, pr1)                in_is_intersect_rect(pr0, pr1) 
    #define  is_zero_rect(pr)                           in_is_zero_rect(pr)  
    #define  is_none_zero_rect(pr)                      in_is_none_zero_rect(pr) 
    #endif  /* _LG_ALONE_VERSION_ */


#ifdef  __cplusplus
}
#endif

#endif  /* __LGUI_RECT_OPS_HEADER__ */
