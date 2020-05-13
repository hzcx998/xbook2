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

#include  <d2_pixel.h>
#include  <d2_line.h>
#include  <screen.h>

#include  <d2_triangle.h>

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif


#ifdef	_LG_TRIANGLE_
int   in_fill_triangle(HDC hdc, int left, int top, int right, int bottom, unsigned int dir)
{
    /* width of rectangle around line */
    int  xdelta = 0;

    /* height of rectangle around line */
    int  ydelta = 0;

    /* increment for moving x coordinate */
    int  xinc = 0;

    /* increment for moving y coordinate */
    int  yinc = 0;

    /* current remainder */
    int  rem = 0;

    /* Temp var */
    int  x = 0;
    int  y = 0;  



    if ( hdc == NULL )
        return  -1;


    xdelta = right - left;
    ydelta = bottom - top;

    if ( xdelta < 0 ) 
        xdelta = -xdelta;

    if ( ydelta < 0 ) 
        ydelta = -ydelta;

    xinc = ( right > left ) ? 1 : -1;
    yinc = ( bottom > top ) ? 1 : -1;


    (lscrn.output_sequence_start());

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + right, hdc->rect.top + bottom);
    #endif
    #endif 

    #ifdef  _LG_WINDOW_
    if (in_init_max_output_rect(hdc) < 1)
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

    
    if ( top == bottom ) 
    {
        in_paint_h_line_no_clip(hdc, left, right, top);
        #ifdef  _LG_WINDOW_
        continue;
        #else

        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        in_cursor_maybe_refresh( );
        #endif
        #endif
        (lscrn.output_sequence_end)();
        return  1;

        #endif
    } 

    if ( left == right ) 
    {
        in_paint_v_line_no_clip(hdc, top, bottom, left);
        #ifdef  _LG_WINDOW_
        continue;
        #else

        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        in_cursor_maybe_refresh( );
        #endif
        #endif

        (lscrn.output_sequence_end)();
        return  1;
        #endif
    } 

    x = left;
    y = top;

    if ( xdelta >= ydelta ) 
    {
        rem = xdelta >> 1;
        while (x != right) 
        {
            x   += xinc;
            rem += ydelta;
            if ( rem >= xdelta ) 
            {
                rem -= xdelta;
                y   += yinc;
            }

            if ( (hdc->pen.type) == DOT_PEN_TYPE )
            {
                if ( (x&0x01) == 0x01 )
                    continue;
            }

            if ( dir == TRIANGLE_LEFT )
                in_paint_h_line_no_clip(hdc, right, x, y);
            else if ( dir == TRIANGLE_RIGHT )
                in_paint_h_line_no_clip(hdc, left, x, y);
            else if ( dir == TRIANGLE_UP )
                in_paint_v_line_no_clip(hdc, y,top, x);
            else if ( dir == TRIANGLE_DOWN )
                in_paint_v_line_no_clip(hdc, y, top, x);

        }
    } else {
        rem = ydelta >> 1;
        while ( y != bottom ) 
        {
            y   += yinc;
            rem += xdelta;
            if ( rem >= ydelta ) 
            {
                rem -= ydelta;
                x   += xinc;
            }

            if ( (hdc->pen.type) == DOT_PEN_TYPE )
            {
                if ( (x&0x01) == 0x01 )
                    continue;
            }

            if ( dir == TRIANGLE_LEFT )
                in_paint_h_line_no_clip(hdc, right, x, y);
            else if ( dir == TRIANGLE_RIGHT )
                in_paint_h_line_no_clip(hdc, left, x, y);
            else if ( dir == TRIANGLE_UP )
                in_paint_v_line_no_clip(hdc, y,top, x);
            else if ( dir == TRIANGLE_DOWN )
                in_paint_v_line_no_clip(hdc, y, top, x);

        }
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
int  fill_triangle(HDC hdc, int left, int top, int right, int bottom, unsigned int dir)
{
    int  ret = 0;

    gui_lock( );
    ret =  in_fill_triangle(hdc, left, top, right, bottom, dir);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_TRIANGLE_ */
