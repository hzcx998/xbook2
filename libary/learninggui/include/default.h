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

#ifndef  __LGUI_BASIC_DEFAULT_HEADER__
#define  __LGUI_BASIC_DEFAULT_HEADER__        1

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


#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal function */
    int  in_hdc_basic_init(void);

    int  in_hdc_basic_set_default(HDC hdc);

    #ifdef  _LG_WINDOW_
    int  in_hdc_window_set_default(HDC hdc);
    int  in_hdc_client_set_default(HDC hdc);
    #endif
    /* Internal funciton ok */


    #ifdef  _LG_FONT_
    int  in_basic_set_default_font(const void *font);
    #endif


    #ifndef  _LG_ALONE_VERSION_

    #ifdef  _LG_FONT_
    int  basic_set_default_font(const void *font);
    #endif

    #else  /* _LG_ALONE_VERSION_ */

    #ifdef  _LG_FONT_
    #define  basic_set_default_font(font)                     in_basic_set_default_font(font)
    #endif

    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif
    
#endif  /* __LGUI_BASIC_DEFAULT_HEADER__ */
