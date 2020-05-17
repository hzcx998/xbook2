/*   
 *  Copyright (C) 2011- 2019 Rao Youkun(960747373@qq.com)
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

#ifndef  __LGUI_BANK_HEADER__
#define  __LGUI_BANK_HEADER__        1

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


    int  in_gui_hbank(HDC hdc, int left, int right, int top, int bank_x0, int bank_y0, void *gui_bank); 
    int  in_gui_vbank(HDC hdc, int left, int top, int bottom, int bank_x0, int bank_y0, void *gui_bank);
    int  in_gui_bank_fill(HDC hdc, int left, int top, int right, int bottom, int bank_x0, int bank_y0, void *gui_bank);


    #ifndef  _LG_ALONE_VERSION_

    int  gui_hbank(HDC hdc, int left, int right, int top, int bank_x0, int bank_y0, void *gui_bank); 
    int  gui_vbank(HDC hdc, int left, int top, int bottom, int bank_x0, int bank_y0, void *gui_bank);
    int  gui_bank_fill(HDC hdc, int left, int top, int right, int bottom, int bank_x0, int bank_y0, void *gui_bank);

    #else  /* _LG_ALONE_VERSION_ */

    #define  gui_hbank(hdc,left,right,top,bank_x0,bank_y0,gui_bank)      in_gui_hbank(hdc,left,right,top,bank_x0,bank_y0,gui_bank)
    #define  gui_vbank(hdc,left,top,bottom,bank_x0,bank_y0,gui_bank)     in_gui_vbank(hdc, left,top,bottom,bank_x0,bank_y0,gui_bank)
    #define  gui_bank_fill(hdc,left,top,right,bottom,bank_x0,bank_y0,gui_bank)  in_gui_bank_fill(hdc,left,top,right,bank_x0,bank_y0,bottom,gui_bank)

    #endif  /* _LG_ALONE_VERSION_ */


#ifdef  __cplusplus
}
#endif
    

#endif  /* __LGUI_BANK_HEADER__ */
