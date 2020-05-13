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

#ifndef  __LGUI_FONT_CHARSETS_HEADER__
#define  __LGUI_FONT_CHARSETS_HEADER__


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


        
#ifdef _LG_ASCII_LATIN_D0816C_FONT_
#include  "ascii_latin_dot_0816.h"
#endif
        
#ifdef _LG_ASCII_LATIN_D0612C_FONT_
#include  "ascii_latin_dot_0612.h"
#endif

#ifdef  _LG_MULTI_BYTE_CODE_VERSION_
#ifdef  _LG_GB2312_D1616CS_C1_FONT_
#include  "gb2312_dot_1616_song.h"
#endif
#endif
      
#ifdef  _LG_MULTI_BYTE_CODE_VERSION_
#ifdef  _LG_GB2312_D1212CS_C1_FONT_
#include  "gb2312_dot_1212_song.h"
#endif
#endif
            
#ifdef  _LG_CJK_UNIFIED_D1616CS_FONT_
#include  "uni_cjk_unified_dot_1616_song.h"
#endif

#ifdef  _LG_UNICODE_VERSION_
#ifdef  _LG_HZ1616_PUNCTUATION_
#include  "hz1616_punctuation.h"
#endif
#endif

#ifdef  _LG_CJK_UNIFIED_D1212CS_FONT_
#include  "uni_cjk_unified_dot_1212_song.h"
#endif

#ifdef  _LG_UNICODE_VERSION_
#ifdef  _LG_HZ1212_PUNCTUATION_
#include  "hz1212_punctuation.h"
#endif
#endif

#endif   /* __LGUI_FONT_CHARSETS_HEADER__ */
