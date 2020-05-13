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
#include  <screen.h>

#include  <image_bitmap.h>


#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif


#ifdef  _LG_BITMAP_
static  int  in_check_bitmap(const void *bitmap)
{
    return  1;
}

int  in_bitmap_fill(HDC hdc, int x, int y, const void *bitmap)
{
    const GUI_BITMAP  *pdata = (GUI_BITMAP *)bitmap;
    unsigned int      raw_width;
    unsigned int      raw_height;
    unsigned int      line_char_num = 0;
    unsigned int      offset  = 0;
             int      cur_x = 0;
             int      cur_y = 0;


    #if  defined(_LG_1_BIT_BITMAP_) || defined(_LG_2_BIT_BITMAP_) || defined(_LG_4_BIT_BITMAP_)
    unsigned char     bit     = 0;
    #endif

    #if  defined(_LG_1_BIT_BITMAP_) || defined(_LG_2_BIT_BITMAP_)
    unsigned char     data    = 0;
    #endif

    unsigned char     index   = 0;

    SCREEN_COLOR      screen_color;

    GUI_COLOR         color;

    #ifdef  _LG_16_BIT_BITMAP_
    unsigned char     low_color  = 0;
    unsigned char     high_color = 0;
    #endif

    #if  defined(_LG_24_BIT_BITMAP_) || defined(_LG_32_BIT_BITMAP_)
    unsigned char     red_color    = 0;
    unsigned char     green_color  = 0;
    unsigned char     blue_color   = 0;
    #endif

             int      i       = 0;
             int      j       = 0;

             int      ret     = 0;


    if ( pdata == NULL )
        return  -1;


    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + x, hdc->rect.top + y, hdc->rect.right, hdc->rect.bottom);
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
    #endif  /* _LG_WINDOW_ */


    ret =  in_check_bitmap( (void *)pdata );
    if ( ret < 0 )
    {
        (lscrn.output_sequence_end)();
        return  -1;
    }

    raw_width     = pdata->width;
    raw_height    = pdata->height;
    line_char_num = pdata->bytes_per_line;

    switch ( pdata->bits_per_pixel )
    {
        /* palette color */
        #ifdef  _LG_1_BIT_BITMAP_
        case  1:
            for ( j = 0; j < raw_height; j++ )
            {
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j;
                    for ( i = 0; i < raw_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset = line_char_num*(raw_height-j-1) + (i/8);
                            bit    = (i%8);

                            data   = *((pdata->data)+offset);
                            data   = (data<<bit)&0x80;
                            if ( data == 0x80 )
                                index = 1;
                            else
                                index = 0;

                            color = pdata->palette->entries[index];
                            screen_color= (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                       }
                   }
                }
            }
            break;
        #endif  /* _LG_1_BIT_BITMAP_ */

        #ifdef  _LG_2_BIT_BITMAP_
        case  2:
            break;
        #endif  /* _LG_2_BIT_BITMAP_ */

        #ifdef  _LG_4_BIT_BITMAP_
        case  4:
            for ( j = 0; j < raw_height; j++ )
            {
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j;
                    for ( i = 0; i < raw_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset = line_char_num*(raw_height-j-1) + (i/2);
                            bit    = (i%2);

                            index  = *((pdata->data)+offset);
                            if ( bit > 0 )
                                index &= 0x0F;
                            else
                                index = (index>>4)&0x0F;

                            color  = pdata->palette->entries[index];
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                       }
                   }
                }
            }
            break;
        #endif  /* _LG_4_BIT_BITMAP_ */

        #ifdef  _LG_8_BIT_BITMAP_
        case  8:
            for ( j = 0; j < raw_height; j++ )
            {
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j;
                    for ( i = 0; i < raw_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset = line_char_num*(raw_height-j-1) + i;
                            index  = *((pdata->data)+offset);

                            color  = pdata->palette->entries[index];
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                    }
                }
            }
            break;
        #endif  /* _LG_8_BIT_BITMAP_ */


        /* true color */
        #ifdef  _LG_16_BIT_BITMAP_
        case  16:
            for ( j = 0; j < raw_height; j++ )
            {
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j;
                    for ( i = 0; i < raw_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;
                            offset     = line_char_num*(raw_height-j-1) + 2*i;
                            low_color  = *((pdata->data)+offset+0);
                            high_color = *((pdata->data)+offset+1);

                            /* 16-bit screen color to  current screen color */
                            color  = (high_color << 8) | low_color; 
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                    }
                }
            }
            break;
        #endif  /* _LG_16_BIT_BITMAP_ */

        #ifdef  _LG_24_BIT_BITMAP_
        case  24:
            for ( j = 0; j < raw_height; j++ )
            {
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j;
                    for ( i = 0; i < raw_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset      = line_char_num*(raw_height-j-1) + 3*i;

                            blue_color  = *((pdata->data)+offset + 0);
                            green_color = *((pdata->data)+offset + 1);
                            red_color   = *((pdata->data)+offset + 2);

                            color = (red_color<<16)|(green_color<<8)|blue_color;
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                    }
                }
            }
            break;
        #endif  /* _LG_24_BIT_BITMAP_ */

        #ifdef  _LG_32_BIT_BITMAP_
        case  32:
            for ( j = 0; j < raw_height; j++ )
            {
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j;
                    for ( i = 0; i < raw_width; i++ )
                    {                     
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset      = line_char_num*(raw_height-j-1) + 4*i;

                            blue_color  = *((pdata->data)+offset + 0);
                            green_color = *((pdata->data)+offset + 1);
                            red_color   = *((pdata->data)+offset + 2);
                            /*
                            alpha       = *((pdata->data)+offset + 3);
                            */

                            /* alpha ?? */
                            color = (red_color<<16)|(green_color<<8)|blue_color;
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                    }
                }
            }
            break;
        #endif  /* _LG_32_BIT_BITMAP_ */

        default:
            break;

    }

    #ifdef  _LG_WINDOW_
    }
    #endif  /* _LG_WINDOW_ */

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  bitmap_fill(HDC hdc, int x, int y, const void *bitmap)
{
    int  ret = 0;

    gui_lock( );
    ret = in_bitmap_fill(hdc, x, y, bitmap);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */
 
 
#ifdef   _LG_FILL_BITMAP_EXTENSION_

#define  muldiv64(m1, m2, d)          (((long long int)(m1*m2))/d) 

/* This is a DDA-based algorithm. */
/* Iteration over target bitmap. */
int  in_bitmap_fill_rect(HDC hdc, const void *rect, const void *bitmap)
{
    const GUI_BITMAP  *pdata = (GUI_BITMAP *)bitmap;
    unsigned int      raw_width;
    unsigned int      raw_height;
    unsigned int      real_width;
    unsigned int      real_height;
    unsigned int      line_char_num = 0;
    unsigned int      offset  = 0;

    #if  defined(_LG_1_BIT_BITMAP_) || defined(_LG_2_BIT_BITMAP_) || defined(_LG_4_BIT_BITMAP_)
    unsigned char     bit     = 0;
    #endif

    #if  defined(_LG_1_BIT_BITMAP_) || defined(_LG_2_BIT_BITMAP_)
    unsigned char     data    = 0;
    #endif

    unsigned char     index   = 0;
    GUI_RECT          *p      = (GUI_RECT *)rect;
             int      x = p->left;
             int      y = p->top;
             int      cur_x = 0;
             int      cur_y = 0;

    SCREEN_COLOR      screen_color;

    GUI_COLOR         color;

    #ifdef  _LG_16_BIT_BITMAP_
    #ifdef  _LG_SCREEN_TO_SCREEN_COLOR_
    unsigned char     low_color  = 0;
    unsigned char     high_color = 0;
    #endif
    #endif

    #if  defined(_LG_24_BIT_BITMAP_) || defined(_LG_32_BIT_BITMAP_)
    unsigned char     red_color    = 0;
    unsigned char     green_color  = 0;
    unsigned char     blue_color   = 0;
    #endif

    #ifdef  _LG_32_BIT_BITMAP_
    unsigned char     alpha    = 0;
    #endif

             int      i       = 0;
             int      j       = 0;

             int      ret     = 0;

             int      xfactor = 0;
             int      yfactor = 0;
                        
             int      sx      = 0;
             int      sy      = 0;



    if ( pdata == NULL )
        return  -1;


    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + p->left, hdc->rect.top + p->top, hdc->rect.left + p->right, hdc->rect.top + p->bottom);
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
    #endif  /* _LG_WINDOW_ */


    ret =  in_check_bitmap( (void *)pdata );
    if ( ret < 0 )
    {
        (lscrn.output_sequence_end)();
        return  -1;
    }

    raw_width     = pdata->width;
    raw_height    = pdata->height;
    line_char_num = pdata->bytes_per_line;

    real_width    = p->right - p->left;
    real_height   = p->bottom - p->top;

    /* scaled by 65536 */
    xfactor = muldiv64(raw_width,  65536, real_width); 
    /* scaled by 65536 */
    yfactor = muldiv64(raw_height, 65536, real_height);        

    switch ( pdata->bits_per_pixel )
    {
        /* palette color */
        #ifdef  _LG_1_BIT_BITMAP_
        case  1:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++ )
            {
                sx = 0;
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j; 
                    for ( i = 0; i < real_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16)/8;
                            if ( offset > (line_char_num * raw_height - 1) )
                                offset  = (line_char_num * raw_height - 1);

                            bit    = (i%8);

                            data   = *((pdata->data)+offset);
                            data   = (data<<bit)&0x80;
                            if ( data == 0x80 )
                                index = 1;
                            else
                                index = 0;

                            color = pdata->palette->entries[index];
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_1_BIT_BITMAP_ */

        #ifdef  _LG_2_BIT_BITMAP_
        case  2:
            break;
        #endif  /* _LG_2_BIT_BITMAP_ */

        #ifdef  _LG_4_BIT_BITMAP_
        case  4:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++ )
            {
                sx = 0;
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j; 
                    for ( i = 0; i < real_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16)/2;
                            if ( offset > (line_char_num * raw_height - 1) )
                                offset  = (line_char_num * raw_height - 1);

                            bit    = (i%2);

                            index  = *((pdata->data)+offset);
                            if ( bit > 0 )
                                index &= 0x0F;
                            else
                                index = (index>>4)&0x0F;

                            color  = pdata->palette->entries[index];
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_4_BIT_BITMAP_ */

        #ifdef  _LG_8_BIT_BITMAP_
        case  8:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++ )
            {
                sx = 0;
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j; 
                    for ( i = 0; i < real_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16);
                            if ( offset > (line_char_num * raw_height - 1) )
                                offset  = (line_char_num * raw_height - 1);

                            index  = *((pdata->data)+offset);

                            color  = pdata->palette->entries[index];
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_8_BIT_BITMAP_ */


        /* true color */
        #ifdef  _LG_16_BIT_BITMAP_
        #ifdef  _LG_SCREEN_TO_SCREEN_COLOR_
        case  16:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++ )
            {
                sx = 0;
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j; 
                    for ( i = 0; i < real_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16)*2;
                            if ( offset > (line_char_num * raw_height - 2) )
                                offset  = (line_char_num * raw_height - 2);

                            low_color  = *((pdata->data)+offset+0);
                            high_color = *((pdata->data)+offset+1);

                            /* 16-bit screen color to  current screen color */ 
                            color  = (high_color << 8) | low_color; 
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_SCREEN_TO_SCREEN_COLOR_ */
        #endif  /* _LG_16_BIT_BITMAP_ */

        #ifdef  _LG_24_BIT_BITMAP_
        case  24:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++)
            {
                sx = 0;
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j;
                    for ( i = 0; i < real_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16)*3;
                            if ( offset > (line_char_num * raw_height - 3) )
                                offset  = (line_char_num * raw_height - 3);

                            blue_color  = *((pdata->data)+offset + 0);
                            green_color = *((pdata->data)+offset + 1);
                            red_color   = *((pdata->data)+offset + 2);

                            color = (red_color<<16)|(green_color<<8)|blue_color;
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_24_BIT_BITMAP_ */

        #ifdef  _LG_32_BIT_BITMAP_
        case  32:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++)
            {
                sx = 0;
                if ( ((y+j)>=0)&&((y+j)<GUI_RECTH(&(hdc->rect))) )
                {
                    cur_y = hdc->rect.top+y+j;
                    for ( i = 0; i < real_width; i++ )
                    {
                        if ( ((x+i)>=0)&&((x+i)<GUI_RECTW(&(hdc->rect))) )
                        {
                            cur_x = hdc->rect.left + x + i;

                            offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16)*4;
                            if ( offset > (line_char_num * raw_height - 4) )
                                offset  = (line_char_num * raw_height - 4);

                            blue_color  = *((pdata->data)+offset + 0);
                            green_color = *((pdata->data)+offset + 1);
                            red_color   = *((pdata->data)+offset + 2);
                                alpha   = *((pdata->data)+offset + 3);

                            if ( alpha )
                            {
                                color = (red_color<<16)|(green_color<<8)|blue_color;
                                screen_color = (lscrn.gui_to_screen_color)(color);
                                in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                            }
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_32_BIT_BITMAP_ */

        default:
            break;

    }

    #ifdef  _LG_WINDOW_
    }
    #endif  /* _LG_WINDOW_ */

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif
 
    (lscrn.output_sequence_end)();

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  bitmap_fill_rect(HDC hdc, const void *rect, const void *bitmap)
{
    int  ret = 0;

    gui_lock( );
    ret = in_bitmap_fill_rect(hdc, rect, bitmap);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_FILL_BITMAP_EXTENSION_ */


        
#ifdef   _LG_BITMAP_FILE_

static  volatile  GUI_BITMAP  lfile = { 0 };


int  in_bitmap_file_fill(HDC hdc, int x, int y, const char *file)
{

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  bitmap_file_fill(HDC hdc, int x, int y, const char *file)
{
    int  ret = 0;

    gui_lock( );
    ret = in_bitmap_file_fill(hdc, x, y, file);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */



#ifdef   _LG_FILL_BITMAP_EXTENSION_

int  in_bitmap_file_fill_rect(HDC hdc, const void *rect, const char *file)
{

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  bitmap_file_fill_rect(HDC hdc, const void *rect, const char *file)
{
    int  ret = 0;

    gui_lock( );
    ret = in_bitmap_file_fill_rect(hdc, rect, file);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_FILL_BITMAP_EXTENSION_ */

 
#endif  /* _LG_BITMAP_FILE_ */

#endif    /* _LG_BITMAP_ */
