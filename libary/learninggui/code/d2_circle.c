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

#include  <lock.h>
#include  <cursor.h>

#include  <dc.h>

#include  <d2_pixel.h>
#include  <d2_line.h>
#include  <screen.h>

#include  <d2_circle.h>

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif



#ifdef	_LG_CIRCLE_
int  in_circle(HDC hdc, int x, int y, unsigned int r)
{
    int  tx = 0;
    int  ty = r;
    int  d  = 3-2*r;


    GUI_COLOR       gui_color;
    SCREEN_COLOR    screen_color;

  
    gui_color = in_hdc_get_fore_color(hdc);
    screen_color = (lscrn.gui_to_screen_color(gui_color));

    (lscrn.output_sequence_start)();


    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + x - r, hdc->rect.top + y - r, hdc->rect.left + x + r, hdc->rect.top + y + r);
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

    while ( tx < ty )
    {
        /* Less than 45 degrees. paint tow points */
        in_output_screen_pixel_abs(hdc, x-ty+hdc->rect.left, y-tx+hdc->rect.top, screen_color);
        in_output_screen_pixel_abs(hdc, x+ty+hdc->rect.left, y-tx+hdc->rect.top, screen_color);
        /* Exclude repeat point */
        if ( tx != 0 )
        {
            in_output_screen_pixel_abs(hdc, x-ty+hdc->rect.left, y+tx+hdc->rect.top, screen_color);
            in_output_screen_pixel_abs(hdc, x+ty+hdc->rect.left, y+tx+hdc->rect.top, screen_color);
        }

        if ( d < 0 )       /* Up point */
        {
            d += 4*tx + 6;
        } else {           /* Down point */
            /* More than 45 degrees. paint tow points */
            in_output_screen_pixel_abs(hdc, x-tx+hdc->rect.left, y-ty+hdc->rect.top, screen_color);
            in_output_screen_pixel_abs(hdc, x+tx+hdc->rect.left, y-ty+hdc->rect.top, screen_color);

            in_output_screen_pixel_abs(hdc, x-tx+hdc->rect.left, y+ty+hdc->rect.top, screen_color);
            in_output_screen_pixel_abs(hdc, x+tx+hdc->rect.left, y+ty+hdc->rect.top, screen_color);

            d += 4*(tx-ty) + 10;
            ty--;
        }

        tx++;

    }

    /* Equal 45 degrees. Draw tow points */
    if ( tx == ty )
    {
        in_output_screen_pixel_abs(hdc, x-ty+hdc->rect.left, y-tx+hdc->rect.top, screen_color);
        in_output_screen_pixel_abs(hdc, x+ty+hdc->rect.left, y-tx+hdc->rect.top, screen_color);

        in_output_screen_pixel_abs(hdc, x-ty+hdc->rect.left, y+tx+hdc->rect.top, screen_color);
        in_output_screen_pixel_abs(hdc, x+ty+hdc->rect.left, y+tx+hdc->rect.top, screen_color);
    }

    #ifdef  _LG_WINDOW_
    }
    #endif

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  circle(HDC hdc, int x, int y, unsigned int r)
{
    int  ret = 0;

    gui_lock( );
    ret = in_circle(hdc, x, y, r);
    gui_unlock( );

    return  ret;
}
#endif

int  in_circle_fill(HDC hdc, int x, int y, unsigned int r)
{
    int  tx = 0;
    int  ty = r;
    int  d  = 3-2*r;


    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + x - r, hdc->rect.top + y - r, hdc->rect.left + x + r, hdc->rect.top + y + r);
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

    while ( tx < ty )
    {
        /* Less than 45 degrees. paint tow horizon lines */
        in_paint_h_line_no_clip(hdc, x-ty, x+ty, y-tx);
        /* Exclude repeat line */
        if ( tx != 0 )
            in_paint_h_line_no_clip(hdc, x-ty, x+ty, y+tx);


        if ( d < 0 )       /* Up point */
        {
            d += 4*tx + 6;
        } else {           /* Down point */
            /* More than 45 degrees. paint tow horizon lines */
            in_paint_h_line_no_clip(hdc, x-tx, x+tx, y-ty);
            in_paint_h_line_no_clip(hdc, x-tx, x+tx, y+ty);

            d += 4*(tx-ty) + 10;
            ty--;
        }

        tx++;

    }

    /* Equal 45 degrees. Draw h-line */
    if ( tx == ty )
    {
        in_paint_h_line_no_clip(hdc, x-ty, x+ty, y-tx);
        in_paint_h_line_no_clip(hdc, x-ty, x+ty, y+tx);
    }

    #ifdef  _LG_WINDOW_
    }
    #endif

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  circle_fill(HDC hdc, int x, int y, unsigned int r)
{
    int  ret = 0;

    gui_lock( );
    ret = in_circle_fill(hdc, x, y, r);
    gui_unlock( );

    return  ret;
}
#endif

#endif	/* _LG_CIRCLE_ */
