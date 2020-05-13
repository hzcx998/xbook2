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

#ifndef  __LGUI_PALETTE_HEADER__
#define  __LGUI_PALETTE_HEADER__        1

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



struct  _GUI_PALETTE 
{
  UINT       num; 
  GUI_COLOR  *entries; 
};
typedef  struct  _GUI_PALETTE    GUI_PALETTE; 


#ifdef  _LG_COLOR_

#ifdef  __cplusplus
extern  "C"
{
#endif

    extern  GUI_PALETTE  lpal;

    #ifdef  _LG_COMMON_PALETTE_
    extern  GUI_VAR_CONST  GUI_PALETTE  l1pal;
    extern  GUI_VAR_CONST  GUI_PALETTE  l2pal;
    extern  GUI_VAR_CONST  GUI_PALETTE  l4pal;
    extern  GUI_VAR_CONST  GUI_PALETTE  l8pal;
    #endif  /* _LG_COMMON_PALETTE_ */


    #ifdef  _LG_PALETTE_ROUTINE_


    int  in_palette_set(void *palette);
    int  in_palette_get(void *palette);

    #ifdef  _LG_RESTORE_SYSTEM_PALETTE_
    int  in_palette_restore_system(void);
    #endif


     #ifndef  _LG_ALONE_VERSION_

        int  palette_set(void *palette);
        int  palette_get(void *palette);

        #ifdef  _LG_RESTORE_SYSTEM_PALETTE_
        int  palette_restore_system(void);
        #endif

    #else  /* _LG_ALONE_VERSION_ */

        #define  palette_set(palette)           in_palette_set(palette)
        #define  palette_get(palette)           in_palette_get(palette)

        #ifdef  _LG_RESTORE_SYSTEM_PALETTE_
        #define  palette_restore_system()       in_palette_restore_system()
        #endif

    #endif  /* _LG_ALONE_VERSION_ */

    #endif  /* _LG_PALETTE_ROUTINE_ */

#ifdef  __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _LG_COLOR_ */

#endif  /* __LGUI_PALETTE_HEADER__ */
