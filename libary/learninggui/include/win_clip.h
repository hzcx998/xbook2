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

#ifndef  __LGUI_WIN_CLIP_HEADER__
#define  __LGUI_WIN_CLIP_HEADER__

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


#ifdef  _LG_WINDOW_
#include  <win_type_widget.h>
#endif



struct  _CLIP_RECT
{
    GUI_RECT      output_rect;
    unsigned int  output_flag;
    GUI_RECT      cur_clip_rect;
    unsigned int  clip_num;
};
typedef  struct _CLIP_RECT    CLIP_RECT;


#ifdef  _LG_WINDOW_

#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal function */
    int  in_init_max_output_rect(HDC hdc);
    int  in_get_current_clip_rect(HDC hdc);

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_CLIP_HEADER__ */
