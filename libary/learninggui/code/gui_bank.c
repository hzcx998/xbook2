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

#include  <stdio.h>

#include  <lock.h>
#include  <cursor.h>

#include  <dc.h>
#include  <d2_pixel.h>
#include  <d2_line.h>
#include  <screen.h>

#include  <gui_bank.h>

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif



/* Absolute coordinate */    
static  int  in_output_screen_hbank_abs(HDC hdc, int left, int right, int top, int bank_x0, int bank_y0, void *gui_bank)
{
    GUI_RECT    rect = { 0 };
    GUI_BANK   *bank = (GUI_BANK *)gui_bank;

    #ifdef  _LG_ALPHA_BLEND_
    int  i = 0;
    #endif


    if ( hdc == NULL ) 
        return  -1;

    if ( bank == NULL)
        return  -1;


    #ifdef  _LG_WINDOW_
    rect = ((HWND)(hdc->hwnd))->common.cur_clip_rect;
    #else
    rect = hdc->rect;
    #endif


    if ( left > rect.right )
        return  -1;
    if ( right < rect.left )
        return  -1;
    if ( top > rect.bottom )
        return  -1;


    if ( left < rect.left )
        left = rect.left;
    if ( left > rect.right )
        left = rect.right;

    if ( right < rect.left )
        right = rect.left;
    if ( right > rect.right )
        right = rect.right;


    if ( top < rect.top )
        top = rect.top;
    if ( top > rect.bottom )
        top = rect.bottom;


    #ifdef  _LG_ALPHA_BLEND_
    if ( (hdc->is_alpha_blend) > 0 )
    {
        for ( i = 0; i < (right - left) + 1; i++ )
        {
            in_output_screen_pixel_abs(hdc,left+i,top,(SCREEN_COLOR)(*(bank->p+i)));
        }

        return  1;
    }
    #endif   


    if ( (lscrn.output_hbank) == NULL )
        return  -1;

    return  (lscrn.output_hbank)(left, right, top, bank_x0, bank_y0, bank);
} 
    
int  in_gui_hbank(HDC hdc, int left, int right, int top, int bank_x0, int bank_y0, void *gui_bank)
{
    int  temp = 0;


    if ( hdc == NULL )
        return  -1;
    if ( gui_bank == NULL )
        return  -1;


    if ( left > right )
    {
        temp  = left;
        left  = right;
        right = temp;
    }


    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + right, hdc->rect.top + top);
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


    if ( (lscrn.is_hbank_accelerate) > 0 )
    {
        in_output_screen_hbank_abs(hdc, left + hdc->rect.left, right + hdc->rect.left, top + hdc->rect.top, bank_x0 + hdc->rect.left, bank_y0 + hdc->rect.top, gui_bank);
    } else {
        in_paint_h_line_no_clip(hdc, left, right, top);
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
int  gui_hbank(HDC hdc, int left, int right, int top, int bank_x0, int bank_y0, void *gui_bank)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_hbank(hdc, left, right, top, bank_x0, bank_y0, gui_bank);
    gui_unlock( );

    return  ret;
}
#endif


/* Absolute coordinate */
static  int  in_output_screen_vbank_abs(HDC hdc, int left, int top, int bottom, int bank_x0, int bank_y0, void *gui_bank)
{
    GUI_RECT    rect = { 0 };
    GUI_BANK   *bank = (GUI_BANK *)gui_bank;

    #ifdef  _LG_ALPHA_BLEND_
    int  i = 0;
    #endif


    if ( hdc == NULL ) 
        return  -1;
    if ( bank == NULL )
        return  -1;


    #ifdef  _LG_WINDOW_
    rect = ((HWND)(hdc->hwnd))->common.cur_clip_rect;
    #else
    rect = hdc->rect;
    #endif


    if ( left > rect.right )
        return  -1;
    if ( top > rect.bottom )
        return  -1;
    if ( bottom < rect.top )
        return  -1;

    if ( left < rect.left )
        left = rect.left;
    if ( left > rect.right )
        left = rect.right;

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
            in_output_screen_pixel_abs(hdc,left,top+i,(SCREEN_COLOR)(*(bank->p+(bank->width)*i)));
        }

        return  1;
    }
    #endif   


    if ( (lscrn.output_vbank) == NULL )
        return  -1;

    return  (lscrn.output_vbank)(left, top, bottom, bank_x0, bank_y0, bank);
}

int  in_gui_vbank(HDC hdc, int left, int top, int bottom, int bank_x0, int bank_y0, void *gui_bank)
{
    int  temp = 0;


    if ( hdc == NULL )
        return  -1;
    if ( gui_bank == NULL )
        return  -1;


    if ( top > bottom )
    {
        temp  = top;
        top   = bottom;
        bottom = temp;
    }


    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + left, hdc->rect.top + bottom);
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


    if ( (lscrn.is_vbank_accelerate) > 0 )
    {
        in_output_screen_vbank_abs(hdc,left+hdc->rect.left,top+hdc->rect.top,bottom+hdc->rect.top, bank_x0+hdc->rect.left, bank_y0+hdc->rect.top, gui_bank);
    } else {
        in_paint_v_line_no_clip(hdc, left, top, bottom);
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
int  gui_vbank(HDC hdc, int left, int top, int bottom, int bank_x0, int bank_y0, void *gui_bank)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_vbank(hdc, left, top, bottom, bank_x0, bank_y0, gui_bank);
    gui_unlock( );

    return  ret;
}
#endif


/* Absolute coordinate */
static  int  in_output_screen_bank_fill_abs(HDC hdc, int left, int top, int right, int bottom, int bank_x0, int bank_y0, void *gui_bank)
{
    GUI_RECT    rect = { 0 };
    GUI_BANK   *bank = (GUI_BANK *)gui_bank;

    #ifdef  _LG_ALPHA_BLEND_
    int  i = 0;
    int  j = 0;
    #endif


    if ( hdc == NULL ) 
        return  -1;
    if ( bank == NULL)
        return  -1;


    #ifdef  _LG_WINDOW_
    rect = ((HWND)(hdc->hwnd))->common.cur_clip_rect;
    #else
    rect = hdc->rect;
    #endif


    if ( left > rect.right )
        return  -1;
    if ( right < rect.left )
        return  -1;
    if ( top > rect.bottom )
        return  -1;
    if ( bottom < rect.top )
        return  -1;


    if ( left < rect.left )
        left = rect.left;
    if ( left > rect.right )
        left = rect.right;

    if ( right < rect.left )
        right = rect.left;
    if ( right > rect.right )
        right = rect.right;


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
        for ( j = 0; j < (bottom - top) + 1; j++ )
        {
            for ( i = 0; i < (right - left) + 1; i++ )
            {
                in_output_screen_pixel_abs(hdc,left+i,top+j,(SCREEN_COLOR)(*(bank->p+j*(bottom-top+1)+i)));
            }
        }

        return  1;
    }
    #endif   


    if ( (lscrn.output_bank_fill) == NULL )
        return  -1;

    return  (lscrn.output_bank_fill)(left, top, right, bottom, bank_x0, bank_y0, bank);
}


int  in_gui_bank_fill(HDC hdc,int left, int top, int right, int bottom, int bank_x0, int bank_y0, void *gui_bank)
{
    int  i    = 0; 
    int  temp = 0;


    if ( hdc == NULL )
        return  -1;
    if ( gui_bank == NULL )
        return  -1;


    if ( left > right )
    {
        temp  = left;
        left  = right;
        right = temp;
    }

    if ( top > bottom )
    {
        temp  = top;
        top   = bottom;
        bottom = temp;
    }


    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + right, hdc->rect.top + bottom);
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


    if ( (lscrn.is_bank_fill_accelerate) > 0 )
    {
        in_output_screen_bank_fill_abs(hdc, left + hdc->rect.left, top + hdc->rect.top, right + hdc->rect.left, bottom + hdc->rect.top, bank_x0, bank_y0, gui_bank);
    } else {
        for (i = top; i < (bottom+1); i++)
            in_paint_h_line_no_clip(hdc, left, right, i);
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
int  gui_bank_fill(HDC hdc,int left, int top, int right, int bottom, int bank_x0, int bank_y0, void *gui_bank)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_bank_fill(hdc, left, top, right, bottom, bank_x0, bank_y0, gui_bank);
    gui_unlock( );

    return  ret;
}
#endif
