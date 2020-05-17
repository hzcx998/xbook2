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

#ifndef  __LGUI_TEXT_OPS_HEADER__
#define  __LGUI_TEXT_OPS_HEADER__

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



#define  TEXT_FRAME_LEFT                           (1<<0)
#define  TEXT_FRAME_TOP                            (1<<1)
#define  TEXT_FRAME_RIGHT                          (1<<2)
#define  TEXT_FRAME_BOTTOM                         (1<<3)


/* Text align format */
#define  LG_TA_LEFT                           (1<<0)
#define  LG_TA_TOP                            (1<<1)
#define  LG_TA_RIGHT                          (1<<2)
#define  LG_TA_BOTTOM                         (1<<3)
#define  LG_TA_HCENTER                        (1<<4)
#define  LG_TA_VCENTER                        (1<<5)
#define  LG_TA_CENTER                         (LG_TA_HCENTER | LG_TA_VCENTER)



#ifdef  _LG_FONT_    

#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_text_out(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter);
    int  in_text_rotate_special(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter, GUI_ROTATE *rotate);
    int  in_text_symmetry_special(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter, GUI_SYMMETRY *symmetry);
    int  in_text_symmetry_rotate(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter, GUI_SYMMETRY_ROTATE *s_r);


    #ifdef  _LG_TEXT_OUT_EXTENSION_
    int  in_text_out_rect(/* HDC hdc */ void *hdc, void *rect, const TCHAR *str, int code_counter, unsigned int format);
    #endif


    #ifndef  _LG_ALONE_VERSION_
    int  text_out(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter);
    int  text_rotate_special(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter, GUI_ROTATE *rotate); 
    int  text_symmetry_special(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter, GUI_SYMMETRY *symmetry);
    int  text_symmetry_rotate(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter, GUI_SYMMETRY_ROTATE *s_r);
    #else
    #define  text_out(hdc, x, y, str, code_counter)    in_text_out(hdc, x, y, str, code_counter)
    #define  text_rotate_special(hdc,x,y,str, code_counter,rotate)   in_text_rotate_special(hdc, x,y, str, code_counter, rotate) 
    #define  text_symmetry_special(hdc,x, y, str, code_counter, symmetry)   in_text_symmetry_special(hdc, x, y, str, code_counter, symmetry)

    #define  text_symmetry_rotate(hdc,x, y, str, code_counter, s_r)   in_text_symmetry_rotate(hdc, x, y, str, code_counter, s_r)
    #endif

    #ifdef  _LG_TEXT_OUT_EXTENSION_
    #ifndef  _LG_ALONE_VERSION_
    int  text_out_rect(/* HDC hdc */ void *hdc, void *rect, const TCHAR *str, int code_counter, unsigned int format);
    #else
    #define  text_out_rect(hdc, rect, str, code_counter, format)     in_text_out_rect(hdc, rect, str, code_counter, format)
    #endif
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_FONT_ */

#endif  /* __LGUI_TEXT_OPS_HEADER__ */
