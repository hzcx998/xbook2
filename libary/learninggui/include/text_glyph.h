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

#ifndef  __LGUI_TEXT_GLYPH_HEADER__
#define  __LGUI_TEXT_GLYPH_HEADER__

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
#ifdef  _LG_TEXT_GLYPH_

#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_get_text_glyph(HDC hdc, const TCHAR *str, unsigned int code_counter, unsigned char *buffer, unsigned int buffer_len, unsigned int *out_len); 

    #ifndef  _LG_ALONE_VERSION_
    int  get_text_glyph(HDC hdc, const TCHAR *str, unsigned int code_counter, unsigned char *buffer, unsigned int buffer_len, unsigned int *out_len); 
    #else
    #define  get_text_glyph(hdc, str, code_counter, buffer, buffer_len, out_len)     in_get_text_glyph(hdc, str, code_counter, buffer, buffer_len, out_len)
    #endif

#ifdef  __cplusplus
}
#endif
    
#endif  /* _LG_TEXT_GLYPH_ */
#endif  /* _LG_FONT_ */

#endif  /* __LGUI_TEXT_GLYPH_HEADER__ */
