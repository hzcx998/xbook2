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
#include  <screen.h>

#include  <d2_line.h>

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif


#ifdef  _LG_LINE_

/* Absolute coordinate */    
static  int  in_output_screen_hline_abs(HDC hdc, int left, int right, int top, SCREEN_COLOR  color)
{
    GUI_RECT    rect = { 0 };
    #ifdef  _LG_ALPHA_BLEND_
    int   i = 0;
    #endif


    if ( hdc == NULL ) 
        return  -1;

 
    #ifdef  _LG_WINDOW_
    rect = ((HWND)(hdc->hwnd))->common.cur_clip_rect;
    #else
    rect = hdc->rect;
    #endif


    if ( top < rect.top )
        return  -1;
    if ( top > rect.bottom )
        return  -1;

    if ( right < rect.left )
        return  -1;
    if ( left > rect.right )
        return  -1;


    if ( left < rect.left )
        left = rect.left;
    if ( left > rect.right )
        left = rect.right;

    if ( right < rect.left )
        right = rect.left;
    if ( right > rect.right )
        right = rect.right;


    #ifdef  _LG_ALPHA_BLEND_
    if ( (hdc->is_alpha_blend) > 0 )
    {
        for ( i = 0; i < (right - left) + 1; i++ )
        {
            in_output_screen_pixel_abs(hdc, left + i, top, color);
        }

        return  1;
    }
    #endif   

    if ( (lscrn.output_hline) == NULL )
        return  -1;

    return  (lscrn.output_hline)(left, right, top, color);
}
    
/* Absolute coordinate */    
static  int  in_output_screen_vline_abs(HDC hdc, int left, int top, int bottom, SCREEN_COLOR  color)
{
    GUI_RECT    rect = { 0 };
    #ifdef  _LG_ALPHA_BLEND_
    int   i = 0;
    #endif

 
    #ifdef  _LG_WINDOW_
    rect = ((HWND)(hdc->hwnd))->common.cur_clip_rect;
    #else
    rect = hdc->rect;
    #endif


    if ( left < rect.left )
        return  -1;
    if ( left > rect.right )
        return  -1;

    if ( bottom < rect.top )
        return  -1;
    if ( top > rect.bottom )
        return  -1;


    if ( top < rect.top )
        top = rect.top;
    if ( top > rect.bottom )
        top = rect.bottom;

    if ( bottom < rect.top )
        bottom = rect.top;
    if ( bottom > rect.bottom )
        bottom = rect.bottom;


    #ifdef  _LG_ALPHA_BLEND_
    if ( (hdc->is_alpha_blend) > 0 )
    {
        for ( i = 0; i < (bottom - top) + 1; i++ )
        {
            in_output_screen_pixel_abs(hdc, top + i, left, color);
        }

        return  1;
    }
    #endif

    if ( (lscrn.output_vline) == NULL )
        return  -1;

    return  (lscrn.output_vline)(left, top, bottom, color);
}


/* Relative coordinate */
int  in_paint_h_line_no_clip(HDC hdc, int left, int right, int top)
{
    GUI_COLOR       gui_color    = GUI_BLACK;
    SCREEN_COLOR    screen_color = GUI_BLACK;
    int             width = 0;
    int             i = 0;
   

    gui_color = in_hdc_get_fore_color(hdc);
    screen_color = (lscrn.gui_to_screen_color)(gui_color);
    if (left == right) 
        return  in_output_screen_pixel_abs(hdc, left+hdc->rect.left, top+hdc->rect.top, screen_color);


    /* ?? */
    if (left > right) 
    {
        i     = right;
        right = left;
        left  = i;
    }


    if ( (lscrn.is_hline_accelerate) > 0 )
        return  in_output_screen_hline_abs(hdc, left + hdc->rect.left, right + hdc->rect.left, top + hdc->rect.top, screen_color);


    width = right - left + 1;
    for (i = 0; i < width; i++)
    {
        if ( (hdc->pen.type) == DOT_PEN_TYPE )
        {
            if ( (i&0x01) == 0x01 )
                continue;
        }
        in_output_screen_pixel_abs(hdc, i+left+hdc->rect.left, top+hdc->rect.top, screen_color);
    }

    return  1;
}

int  in_paint_v_line_no_clip(HDC hdc, int top, int bottom, int left)
{
    GUI_COLOR       gui_color    = GUI_BLACK;
    SCREEN_COLOR    screen_color = GUI_BLACK;
    int             height = 0;
    int             i = 0;
   

    gui_color = in_hdc_get_fore_color(hdc);
    screen_color = (lscrn.gui_to_screen_color)(gui_color);
    if (top == bottom) 
    {
        in_output_screen_pixel_abs(hdc, left+hdc->rect.left, top+hdc->rect.top, screen_color);
        return  1;
    }

    /* ?? */
    if (top > bottom) 
    {
        i      = bottom;
        bottom = top;
        top    = i;
    }

  
    if ( (lscrn.is_vline_accelerate) > 0 )
        return  in_output_screen_vline_abs(hdc, left + hdc->rect.left, top + hdc->rect.top, bottom + hdc->rect.top, screen_color);

 
    height = bottom - top + 1;
    for (i = 0; i < height; i++)
    {
        if ( (hdc->pen.type) == DOT_PEN_TYPE )
        {
            if ( (i&0x01) == 0x01 )
                continue;
        }
        in_output_screen_pixel_abs(hdc, left+hdc->rect.left, i+top+hdc->rect.top, screen_color);
    }

    return  1;
}


/* Bresenham line generator */
int   in_line(HDC hdc, int left, int top, int right, int bottom)
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

    /* Color var */ 
    GUI_COLOR       gui_color     = GUI_BLACK;
    SCREEN_COLOR    screen_color  = GUI_BLACK;


    if ( hdc == NULL )
        return  -1;


    gui_color    = in_hdc_get_fore_color(hdc);
    screen_color = (lscrn.gui_to_screen_color)(gui_color);

    xdelta = right - left;
    ydelta = bottom - top;

    if ( xdelta < 0 ) 
        xdelta = -xdelta;

    if ( ydelta < 0 ) 
        ydelta = -ydelta;

    xinc = ( right > left ) ? 1 : -1;
    yinc = ( bottom > top ) ? 1 : -1;

        
    (lscrn.output_sequence_start)();

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

            in_output_screen_pixel_abs(hdc, x+hdc->rect.left, y+hdc->rect.top, screen_color);
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

            in_output_screen_pixel_abs(hdc, x+hdc->rect.left, y+hdc->rect.top, screen_color);
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
int  line(HDC hdc, int left, int top, int right, int bottom)
{
    int  ret = 0;

    gui_lock( );
    ret = in_line(hdc, left, top, right, bottom);
    gui_unlock( );

    return  ret;
}
#endif


int  in_move_to(HDC hdc, int x, int y)
{
    if ( hdc == NULL )
        return  -1;

    hdc->pen.cur_pos.x = x;
    hdc->pen.cur_pos.y = y;

    return  1;
}
        
#ifndef  _LG_ALONE_VERSION_
int  move_to(HDC hdc, int x, int y)
{
    int  ret = 0;

    gui_lock( );
    ret = in_move_to(hdc, x, y);
    gui_unlock( );

    return  ret;
}
#endif

int  in_line_to(HDC hdc, int x, int y)
{
    return  in_line(hdc, hdc->pen.cur_pos.x, hdc->pen.cur_pos.y, x, y);
}

#ifndef  _LG_ALONE_VERSION_
int  line_to(HDC hdc, int x, int y)
{
    int  ret = 0;

    gui_lock( );
    ret = in_line_to(hdc, x, y);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_LINE_ */
