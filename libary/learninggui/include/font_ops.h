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

#ifndef	__LGUI_FONT_OPS_HEADER__
#define	__LGUI_FONT_OPS_HEADER__

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


#ifdef  _LG_FONT_

#ifdef	__cplusplus
extern  "C"
{
#endif	/* __cplusplus */

    #ifdef  _LG_FONT_ID_
    int  in_get_font_id_by_font_name(const void *font, const UCHAR *font_name, UCHAR *font_id);
    int  in_set_font_id_by_font_name(const void *font, const UCHAR *font_name, UCHAR font_id);
    #endif


    #ifdef  _LG_FONT_ID_
    #ifndef  _LG_ALONE_VERSION_
    int  get_font_id_by_font_name(const void *font, const UCHAR *font_name, UCHAR *font_id);
    int  set_font_id_by_font_name(const void *font, const UCHAR *font_name, UCHAR font_id);
    #else
    #define  get_font_id_by_font_name(font, font_name, font_id)     in_get_font_id_by_font_name(font, font_name, font_id)
    #define  set_font_id_by_font_name(font, font_name, font_id)     in_set_font_id_by_font_name(font, font_name, font_id)
    #endif  /* _LG_ALONE_VERSION_ */
    #endif /* _LG_FONT_ID_ */

#ifdef  __cplusplus
}
#endif	/* __cplusplus */
    
#endif	/* _LG_FONT_ */

#endif	/* __LGUI_FONT_OPS_HEADER__ */
