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

#ifndef  __LGUI_LGMACRO_HEADER__
#define  __LGUI_LGMACRO_HEADER__

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif


#ifndef GUI_MAX
#define GUI_MAX(a, b)           ( ((a)>(b)) ? (a) : (b) )
#endif

#ifndef GUI_MIN
#define GUI_MIN(a, b)           ( ( (a) < (b) ) ? (a) : (b) )
#endif

#ifndef GUI_ABS
#define GUI_ABS(a)              (((a)<0) ? -(a) : (a))
#endif



#ifdef  _LG_UNICODE_VERSION_

#ifdef  _LG_UNICODE_BIG_ENDIAN_
#define    GUI_LF                       0x0A00
#define    GUI_CR                       0x0D00
#define    GUI_TAB                      0x0900

#else

#define    GUI_LF                       0x000A
#define    GUI_CR                       0x000D
#define    GUI_TAB                      0x0009

#endif  /* _LG_UNICODE_BIG_ENDIAN_ */

#else

#define    GUI_LF                       0x0A
#define    GUI_CR                       0x0D
#define    GUI_TAB                      0x09

#endif  /* _LG_UNICODE_VERSION_ */


#define    NULL_CHAR                    0x00


#define    PI                           3.1415926535898


#endif  /* __LGUI_LGMACRO_HEADER__ */
