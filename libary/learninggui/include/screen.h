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

#ifndef  __LGUI_SCREEN_HEADER__
#define  __LGUI_SCREEN_HEADER__

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



#define  NO_REFRESH_SCREEN                0x00
#define  NEED_REFRESH_SCREEN              0x01


struct  _GUI_SCREEN
{
    unsigned int   width;
    unsigned int   height;

    BUINT          is_hline_accelerate;
    BUINT          is_vline_accelerate;
    BUINT          is_rect_fill_accelerate;

    BUINT          is_hbank_accelerate;
    BUINT          is_vbank_accelerate;
    BUINT          is_bank_fill_accelerate;
    BUINT          is_bank_copy_accelerate;

    int            (*open)(void);
    int	           (*close)(void);

    SCREEN_COLOR   (*gui_to_screen_color)(GUI_COLOR  gui_color);
    GUI_COLOR      (*screen_to_gui_color)(SCREEN_COLOR  screen_color);
 
    int	           (*output_sequence_start)(void);
    int            (*output_pixel)(int x, int y, SCREEN_COLOR  color);
    int            (*output_hline)(int left, int right, int top, SCREEN_COLOR  color);
    int            (*output_vline)(int left, int top, int bottom, SCREEN_COLOR  color);
    int            (*output_rect_fill)(int left, int top, int right, int bottom, SCREEN_COLOR  color);
    int            (*output_hbank)(int left, int right, int top, int bank_x0, int bank_y0, void *gui_bank); 
    int            (*output_vbank)(int left, int top, int bottom, int bank_x0, int bank_y0, void *gui_bank);
    int            (*output_bank_fill)(int left, int top, int right, int bottom, int bank_x0, int bank_y0, void *gui_bank);

    int	           (*output_sequence_end)(void);

    int	           (*input_sequence_start)(void);
    int            (*input_pixel)(int x, int y, SCREEN_COLOR  *color);
    int	           (*input_sequence_end)(void);

    int            (*control)(void *p1, void *p2);
    int	           (*on)(void);
    int	           (*off)(void);
    int	           (*reinit)(void);


    #ifndef  _LG_WINDOW_
    int	           (*clear)(SCREEN_COLOR color);
    #endif

    #ifdef  _LG_NEED_REFRESH_SCREEN_
    BUINT          is_refresh;
    unsigned int   refresh_interval; 
    int	           (*refresh)(void);
    #endif
	 
    #ifdef  _LG_PALETTE_ROUTINE_
    BUINT          is_palette;
    void          *palette;
    int	           (*palette_set)(void *palette);
    int	           (*palette_get)(void *palette);
    #endif

};
typedef	struct	_GUI_SCREEN  GUI_SCREEN;



#ifdef  __cplusplus
extern  "C"
{
#endif

    extern  volatile  GUI_SCREEN    lscrn;

    /* Internal function */
    int  in_screen_open(void);
    int  in_screen_close(void);
    /* Internal function end */


    int  in_screen_get_width_height(unsigned int *width, unsigned int *height);
    int  in_screen_control(void *p1, void *p2);
    int  in_screen_on(void);
    int  in_screen_off(void);
    int  in_screen_reinit(void);

    #ifndef  _LG_ALONE_VERSION_
    int  screen_get_width_height(unsigned int *width, unsigned int *height);
    int  screen_control(void *p1, void *p2);
    int  screen_on(void);
    int  screen_off(void);
    int  screen_reinit(void);
    #else
    #define  screen_get_width_height(width, height)  in_screen_get_width_height(width, height)
    #define  screen_control(p1, p2)                  in_screen_control(p1, p2)
    #define  screen_on()                             in_screen_on()
    #define  screen_off()                            in_screen_off()
    #define  screen_reinit()                         in_screen_reinit()
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif


#endif  /* __LGUI_SCREEN_HEADER__ */
