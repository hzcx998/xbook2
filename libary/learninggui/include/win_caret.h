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

#ifndef  __LGUI_WIN_CARET_HEADER__
#define  __LGUI_WIN_CARET_HEADER__        1


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


#ifdef  _LG_WINDOW_
#include  <win_type_widget.h>
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_CARET_    


#ifndef  CARET_FORE_COLOR
#define  CARET_FORE_COLOR                 GUI_BLACK
#endif

    
#ifdef  __cplusplus
extern  "C"
{
#endif

    extern  volatile  unsigned int  lcarfl;
    extern  volatile  unsigned int  lcarin;
    extern  volatile  unsigned int  lcarwi;
    extern  volatile  GUI_RECT      lcarre;


    /*Internal function */
    int  in_caret_deal(void);
    int  in_caret_init_blink(void);


    int  in_caret_show(void);
    int  in_caret_hide(void);
    int  in_caret_get_position(void);
    int  in_caret_set_position(unsigned int index);
    int  in_caret_get_blink_interval(void);
    int  in_caret_set_blink_interval(unsigned int interval);
    int  in_caret_get_width(void);
    int  in_caret_set_width(unsigned int width);


    #ifndef  _LG_ALONE_VERSION_
    int  caret_show(void);
    int  caret_hide(void);
    int  caret_get_position(void);
    int  caret_set_position(unsigned int index);
    int  caret_get_blink_interval(void);
    int  caret_set_blink_interval(unsigned int interval);
    int  caret_get_width(void);
    int  caret_set_width(unsigned int width);
    #else  /* _LG_ALONE_VERSION_ */
    #define  caret_show()                          in_caret_show()
    #define  caret_hide()                          in_caret_hide()
    #define  caret_get_position()                  in_caret_get_postion()
    #define  caret_set_position(index)             in_caret_set_position(index)
    #define  caret_get_blink_interval()            in_caret_get_interval()
    #define  caret_set_blink_interval(interval)    in_caret_set_interval(interval)
    #define  caret_get_width()                     in_caret_get_width()
    #define  caret_set_width(wwidth)               in_caret_set_width(width)
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif
   
#endif  /* _LG_CARET_ */ 
#endif  /* _LG_WINDOW_ */
 
#endif  /* __LGUI_WIN_CARET_HEADER__ */
