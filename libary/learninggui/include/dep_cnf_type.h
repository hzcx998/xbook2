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


#ifndef  __LGUI_DEP_CNF_TYPE_HEADER__
#define  __LGUI_DEP_CNF_TYPE_HEADER__   1


#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif

    
#ifdef  _LG_UNICODE_VERSION_
#define  TCHAR                    UINT16
#else
#define  TCHAR                    char
#endif  /* _LG_UNICODE_VERSION_ */

#endif   /*__LGUI_DEP_CNF_TYPE_HEADER__*/
