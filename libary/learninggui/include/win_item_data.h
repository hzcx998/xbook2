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

#ifndef  __LGUI_WIN_ITEM_DATA_HEADER__
#define  __LGUI_WIN_ITEM_DATA_HEADER__    1

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



/* ITEM_DATA_LIST */
struct  _ITEM_DATA_LIST
{
    /* Memory prev hwnd pointer */
    struct  _ITEM_DATA_LIST  *prev;

    /* Memory next hwnd pointer */
    struct  _ITEM_DATA_LIST  *next;

    /* Item selected flag */
    unsigned int  selected;

    /* Item data len */
    unsigned int  len;

    /* Item data pointer */
    TCHAR  *pdata;
};
typedef	struct	_ITEM_DATA_LIST   ITEM_DATA_LIST;



#endif  /* __LGUI_WIN_ITEM_DATA_HEADER__ */
