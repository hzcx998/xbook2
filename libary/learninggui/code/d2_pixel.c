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

#include  <stdio.h>

#include  <lock.h>
#include  <cursor.h>

#include  <dc.h>
#include  <screen.h>

#include  <d2_pixel.h>

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#include  <win_desktop.h>
#endif


#ifdef  _LG_DC_

/* Absolute coordinate */
/* Only for icon cursor display */
int  in_no_clip_output_screen_pixel_abs(HDC hdc, int x, int y, SCREEN_COLOR  color)
{
    GUI_RECT      rect = { 0 };


    #ifdef  _LG_WINDOW_
    /* win_dc.rect */
    rect = HWND_DESKTOP->common.win_dc.rect;
    #else
    rect = hdc->rect;
    #endif

    if ( x < rect.left )
        return  -1;
    if ( x > rect.right )
        return  -1;
    if ( y < rect.top )
        return  -1;
    if ( y > rect.bottom )
        return  -1;

    return  (lscrn.output_pixel)(x , y, color);
}

/* Absolute coordinate */
int  in_output_screen_pixel_abs(HDC hdc, int x, int y, SCREEN_COLOR  color)
{
    GUI_RECT      rect        = { 0 };
    int           ret         = 0;
    #ifdef  _LG_ALPHA_BLEND_
    SCREEN_COLOR  back_color  = GUI_BLACK;
    SCREEN_COLOR  tmp1_color  = GUI_BLACK;
    SCREEN_COLOR  tmp2_color  = GUI_BLACK;
    SCREEN_COLOR  tmp3_color  = GUI_BLACK;
    SCREEN_COLOR  tmp_color   = GUI_BLACK;
    unsigned char alpha_value = 0;
    #endif


    #ifdef  _LG_WINDOW_
    /* cur_clip_rect */
    rect = ((HWND)(hdc->hwnd))->common.cur_clip_rect;
    #else
    rect = hdc->rect;
    #endif

    if ( x < rect.left )
        return  -1;
    if ( x > rect.right )
        return  -1;
    if ( y < rect.top )
        return  -1;
    if ( y > rect.bottom )
        return  -1;


    #ifdef  _LG_ALPHA_BLEND_
    if ( (hdc->is_alpha_blend) < 1 )
        return  (lscrn.output_pixel)(x , y, color);

    ret = in_point_input_pixel(hdc, x, y, &back_color);
    if ( ret < 1 )
        return  -1;

    tmp_color = 0;
    if ( (hdc->alpha_blend_op_mode) == ALPHA_BLEND_OP_SUBTRACT )
    {
        /* Blue */
        tmp1_color = (back_color&0xFF);  
        tmp2_color = (color&0xFF);  
        if ( tmp1_color > tmp2_color )
            tmp_color |= tmp1_color - tmp2_color;

        /* Green */
        tmp1_color = (back_color&0xFF00);  
        tmp2_color = (color&0xFF00);  
        if ( tmp1_color > tmp2_color )
            tmp_color |= tmp1_color - tmp2_color;

        /* RED */
        tmp1_color = (back_color&0xFF0000);  
        tmp2_color = (color&0xFF0000);  
        if ( tmp1_color > tmp2_color )
            tmp_color |= tmp1_color - tmp2_color;

    } else  if ( (hdc->alpha_blend_op_mode) == ALPHA_BLEND_OP_REVSUBTRACT ) {
        /* Blue */
        tmp1_color = (back_color&0xFF);  
        tmp2_color = (color&0xFF);  
        if ( tmp2_color > tmp1_color )
            tmp_color |= tmp2_color - tmp1_color;

        /* Green */
        tmp1_color = (back_color&0xFF00);  
        tmp2_color = (color&0xFF00);  
        if ( tmp2_color > tmp1_color )
            tmp_color |= tmp2_color - tmp2_color;

        /* RED */
        tmp1_color = (back_color&0xFF0000);  
        tmp2_color = (color&0xFF0000);  
        if ( tmp2_color > tmp1_color )
            tmp_color |= tmp2_color - tmp1_color;
 
    } else  if ( (hdc->alpha_blend_op_mode) == ALPHA_BLEND_OP_MIN ) { 
        /* Blue */
        tmp1_color = (back_color&0xFF);
        tmp2_color = (color&0xFF);
        tmp_color |= GUI_MIN(tmp1_color, tmp2_color);

        /* Green */
        tmp1_color = (back_color&0xFF00);  
        tmp2_color = (color&0xFF00);
        tmp_color |= GUI_MIN(tmp1_color, tmp2_color);
 
        /* RED */
        tmp1_color = (back_color&0xFF0000);  
        tmp2_color = (color&0xFF0000); 
        tmp_color |= GUI_MIN(tmp1_color, tmp2_color);

    } else  if ( (hdc->alpha_blend_op_mode) == ALPHA_BLEND_OP_MAX ) {
        /* Blue */
        tmp1_color = (back_color&0xFF);
        tmp2_color = (color&0xFF);
        tmp_color |= GUI_MAX(tmp1_color, tmp2_color);

        /* Green */
        tmp1_color = (back_color&0xFF00);  
        tmp2_color = (color&0xFF00);
        tmp_color |= GUI_MAX(tmp1_color, tmp2_color);
 
        /* RED */
        tmp1_color = (back_color&0xFF0000);  
        tmp2_color = (color&0xFF0000); 
        tmp_color |= GUI_MAX(tmp1_color, tmp2_color);
    } else {
        /* Default: ALPHA_BLEND_OP_ADD */
        alpha_value = (((hdc->color[0][0])&0xFF000000) >> 24);

        /* Blue */
        tmp1_color = (back_color&0xFF);
        tmp2_color = (color&0xFF);
        tmp_color  = (((tmp1_color*alpha_value) + tmp2_color*(256 - alpha_value)) >> 8);
        tmp_color &= 0xFF;

        /* Green */
        tmp1_color = ((back_color&0xFF00)>>8);  
        tmp2_color = ((color&0xFF00)>>8);
        tmp3_color = (((tmp1_color*alpha_value) + tmp2_color*(256 - alpha_value)) >> 8);
        tmp_color |= ((tmp3_color&0xFF)<<8);
 
        /* RED */
        tmp1_color = ((back_color&0xFF0000)>>16);
        tmp2_color = ((color&0xFF0000)>>16);
        tmp3_color = (((tmp1_color*alpha_value) + tmp2_color*(256 - alpha_value)) >> 8);
        tmp_color |= ((tmp3_color&0xFF)<<16);
    }

    color = tmp_color;
    #endif

    ret =  (lscrn.output_pixel)(x , y, color);

    return  ret;
}


/* App function */
int  in_point_output_pixel(HDC hdc, int x, int y)
{
    GUI_COLOR     gui_color    = GUI_BLACK;
    SCREEN_COLOR  screen_color = GUI_BLACK;
    int           ret          = 0;


    if ( hdc == NULL )
        return  -1;

    gui_color    = in_hdc_get_fore_color(hdc);
    screen_color = (lscrn.gui_to_screen_color)(gui_color);

    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + x, hdc->rect.top + y, hdc->rect.left + x, hdc->rect.top + y);
    #endif
    #endif 

    #ifdef  _LG_WINDOW_
    if ( in_init_max_output_rect(hdc) < 1 )
    {
        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        in_cursor_maybe_refresh( );
        #endif
        #endif
        (lscrn.output_sequence_end)();
        return  -1;
    }

    while ( 1 )
    {
        if ( in_get_current_clip_rect(hdc) < 1 )
            break;
    #endif

        ret = in_output_screen_pixel_abs(hdc, x + hdc->rect.left, y + hdc->rect.top, screen_color);

        #ifdef  _LG_WINDOW_
        if ( ret > 0 )
            break;
        #endif

    #ifdef  _LG_WINDOW_
    }
    #endif

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  point_output_pixel(HDC hdc, int x, int y)
{
    int  ret = 0;

    gui_lock( );
    ret = in_point_output_pixel(hdc, x, y);
    gui_unlock( );

    return  ret;
}
#endif

int  in_point_input_pixel(HDC hdc, int x, int y, GUI_COLOR  *color)
{
    SCREEN_COLOR    screen_color = GUI_BLACK;
              int   ret          = -1;


    if ( color == NULL )
        return  -1;

    if ( (lscrn.input_pixel) == NULL )
        return  -1;

    if (  (x>=0)&&(x<GUI_RECTW(&(hdc->rect)))&& \
          (y>=0)&&(y<GUI_RECTH(&(hdc->rect))) )
    {
        (lscrn.input_pixel)(x - hdc->rect.left , y - hdc->rect.top , &screen_color);
        *color = (lscrn.screen_to_gui_color)(screen_color);
        ret = 1;
    }

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  point_input_pixel(HDC hdc, int x, int y, GUI_COLOR  *color)
{
    int  ret = 0;

    gui_lock( );
    ret = in_point_input_pixel(hdc, x, y, color);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_DC_ */
