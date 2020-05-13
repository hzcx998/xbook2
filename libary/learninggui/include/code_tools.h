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

#ifndef  __LGUI_CODE_TOOLS_HEADER__
#define  __LGUI_CODE_TOOLS_HEADER__

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



#ifdef _LG_TOOLS_


#ifdef _LG_GB2312_TO_UNICODE_

#ifdef  __cplusplus
extern  "C"
{
#endif  /* __cplusplus */

    int  in_gb2312_to_unicode(const unsigned char *gb2312_code, UINT16 *uni_code);

#ifdef  __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _LG_GB2312_TO_UNICODE_ */



#ifdef _LG_UNICODE_TO_GB2312_

#ifdef  __cplusplus
extern  "C"
{
#endif  /* __cplusplus */

    int  in_unicode_to_gb2312(const UINT16 *uni_code, unsigned char *gb2312_code);

#ifdef  __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _LG_UNICODE_TO_GB2312_ */


#endif  /* _LG_TOOLS_ */


#endif  /* __LGUI_CODE_TOOLS_HEADER__ */
