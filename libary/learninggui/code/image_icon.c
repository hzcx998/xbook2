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

#include  <image_icon.h>


#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif


#ifdef  _LG_ICON_
static  int  in_check_icon(const void *icon)
{
    return  1;
}

int  in_icon_fill_flag(HDC hdc, int x, int y, const void *icon, unsigned int cursor_flag)
{
    const GUI_ICON    *pdata = (GUI_ICON *)icon;
    unsigned int      raw_width;
    unsigned int      raw_height;
    unsigned int      line_char_num = 0;
    unsigned int      and_char_num  = 0;
    unsigned int      offset  = 0;
    unsigned int      and_offset = 0;

    #if  defined(_LG_1_BIT_ICON_) || defined(_LG_2_BIT_ICON_) || defined(_LG_4_BIT_ICON_)
    unsigned char     bit     = 0;
    #endif

    #if  defined(_LG_1_BIT_ICON_) || defined(_LG_2_BIT_ICON_)
    unsigned char     data    = 0;
    #endif

    unsigned char     index   = 0;


    SCREEN_COLOR      screen_color;

    GUI_COLOR         color;

    #ifdef  _LG_16_BIT_ICON_
    #ifdef  _LG_SCREEN_TO_SCREEN_COLOR_
    unsigned char     low_color  = 0;
    unsigned char     high_color = 0;
    #endif
    #endif

    #if  defined(_LG_24_BIT_ICON_) || defined(_LG_32_BIT_ICON_)
    unsigned char     red_color    = 0;
    unsigned char     green_color  = 0;
    unsigned char     blue_color   = 0;
    #endif

    #ifdef  _LG_32_BIT_ICON_
    unsigned char     alpha    = 0;
    #endif

    unsigned int      i       = 0;
    unsigned int      j       = 0;
    unsigned char     tchar   = 0;
             int      ret     = 0;



    if ( pdata == NULL )
        return  -1;


    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    if ( cursor_flag < 1 )
        in_cursor_maybe_restore_back_abs(hdc->rect.left + x, hdc->rect.top + y, hdc->rect.right, hdc->rect.bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_
    if ( cursor_flag > 0 )
        goto  START_TO_FILL_ICON;

    if (in_init_max_output_rect(hdc) < 1)
    {
        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        if ( cursor_flag < 1 )
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

    START_TO_FILL_ICON: 
    #endif  /* _LG_WINDOW_ */


    ret =  in_check_icon((void *)pdata);
    if ( ret < 0 )
    {
        (lscrn.output_sequence_end)();
        return  -1;
    }

    raw_width     = pdata->width;
    raw_height    = pdata->height;
    line_char_num = pdata->bytes_per_line;                
    and_char_num  = (raw_width+31)/32*4;

    switch ( pdata->bits_per_pixel )
    {
        /* palette color */
        #ifdef  _LG_1_BIT_ICON_
        case  1:
            for ( j = 0; j < raw_height; j++ )
            {
                for ( i = 0; i < raw_width; i++ )
                {
                    and_offset  = and_char_num*(raw_height-j-1) + i/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;


                    offset = line_char_num*(raw_height-j-1) + (i/8);
                    bit    = (i%8);

                    data   = *((pdata->xor_data)+offset);
                    data   = (data<<bit)&0x80;
                    if ( data == 0x80 )
                        index = 1;
                    else
                        index = 0;

                    color = pdata->palette->entries[index];
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    if ( cursor_flag > 0 )
                    {
                        in_no_clip_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    } else {
                        in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    }
                }
            }
            break;
        #endif  /* _LG_1_BIT_ICON_ */

        #ifdef  _LG_2_BIT_ICON_
        case  2:
            break;
        #endif  /* _LG_2_BIT_ICON_ */

        #ifdef  _LG_4_BIT_ICON_
        case  4:
            for ( j = 0; j < raw_height; j++ )
                for ( i = 0; i < raw_width; i++ )
                {
                    and_offset  = and_char_num*(raw_height-j-1) + i/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;


                    offset = line_char_num*(raw_height-j-1) + (i/2);
                    bit    = (i%2);

                    index  = *((pdata->xor_data)+offset);
                    if ( bit > 0 )
                        index &= 0x0F;
                    else
                        index = (index>>4)&0x0F;

                    color  = pdata->palette->entries[index];
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    if ( cursor_flag > 0 )
                    {
                        in_no_clip_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    } else {
                        in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    }
                }
            break;
        #endif  /* _LG_4_BIT_ICON_ */

        #ifdef  _LG_8_BIT_ICON_
        case  8:
            for ( j = 0; j < raw_height; j++ )
                for ( i = 0; i < raw_width; i++ )
                {
                    and_offset  = and_char_num*(raw_height-j-1) + i/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;


                    offset = line_char_num*(raw_height-j-1) + i;
                    index  = *((pdata->xor_data)+offset);

                    color  = pdata->palette->entries[index];
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    if ( cursor_flag > 0 )
                    {
                        in_no_clip_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    } else {
                        in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    }
                }
            break;
        #endif  /* _LG_8_BIT_ICON_ */

        /* true color */
        #ifdef  _LG_16_BIT_ICON_
        #ifdef  _LG_SCREEN_TO_SCREEN_COLOR_
        case  16:
            for ( j = 0; j < raw_height; j++ )
                for ( i = 0; i < raw_width; i++ )
                {
                    and_offset  = and_char_num*(raw_height-j-1) + i/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;


                    offset     = line_char_num*(raw_height-j-1) + 2*i;

                    low_color  = *((pdata->xor_data)+offset+0);
                    high_color = *((pdata->xor_data)+offset+1);

                    /* 16-bit screen color to  current screen color */ 
                    color  = (high_color << 8) | low_color; 
                    if ( cursor_flag > 0 )
                    {
                        in_no_clip_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    } else {
                        in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    }
                }
            break;
        #endif  /* _LG_SCREEN_TO_SCREEN_COLOR_ */
        #endif  /* _LG_16_BIT_ICON_ */

        #ifdef  _LG_24_BIT_ICON_
        case  24:
            for ( j = 0; j < raw_height; j++ )
                for ( i = 0; i < raw_width; i++ )
                {
                    and_offset  = and_char_num*(raw_height-j-1) + i/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;


                    offset      = line_char_num*(raw_height-j-1) + 3*i;

                    blue_color  = *((pdata->xor_data)+offset + 0);
                    green_color = *((pdata->xor_data)+offset + 1);
                    red_color   = *((pdata->xor_data)+offset + 2);

                    color = (red_color<<16)|(green_color<<8)|blue_color;
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    if ( cursor_flag > 0 )
                    {
                        in_no_clip_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    } else {
                        in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    }
                }
            break;
        #endif  /* _LG_24_BIT_ICON_ */

        #ifdef  _LG_32_BIT_ICON_
        case  32:
            for ( j = 0; j < raw_height; j++ )
                for ( i = 0; i < raw_width; i++ )
                {
                    and_offset  = and_char_num*(raw_height-j-1) + i/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;

                    offset      = line_char_num*(raw_height-j-1) + 4*i;

                    blue_color  = *((pdata->xor_data)+offset + 0);
                    green_color = *((pdata->xor_data)+offset + 1);
                    red_color   = *((pdata->xor_data)+offset + 2);
                    alpha       = *((pdata->xor_data)+offset + 3);
                    if ( alpha ==  0 )
                        continue;

                    color = (red_color<<16)|(green_color<<8)|blue_color;
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    if ( cursor_flag > 0 )
                    {
                        in_no_clip_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    } else {
                        in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    }
                }
            break;
        #endif  /* _LG_32_BIT_ICON_ */

        default:
            break;

    }

    #ifdef  _LG_WINDOW_
    if ( cursor_flag > 0 )
        break;

    }
    #endif  /* _LG_WINDOW_ */

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    if ( cursor_flag < 1 )
        in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  icon_fill(HDC hdc, int x, int y, const void *icon)
{
    int   ret = 0;

    gui_lock( );
    ret = in_icon_fill_flag(hdc, x, y, icon, 0);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


#ifdef   _LG_FILL_ICON_EXTENSION_

#define  muldiv64(m1, m2, d)          (((long long int)(m1*m2))/d) 

/* This is a DDA-based algorithm. */
/* Iteration over target bitmap. */
int  in_icon_fill_rect(HDC hdc, const void *rect, const void *icon)
{
    const GUI_ICON    *pdata = (GUI_ICON *)icon;
    unsigned int      raw_width;
    unsigned int      raw_height;
    unsigned int      real_width;
    unsigned int      real_height;
    unsigned int      line_char_num = 0;
    unsigned int      and_char_num  = 0;
    unsigned int      offset  = 0;
    unsigned int      and_offset = 0;

    #if  defined(_LG_1_BIT_ICON_) || defined(_LG_2_BIT_ICON_) || defined(_LG_4_BIT_ICON_)
    unsigned char     bit     = 0;
    #endif

    #if  defined(_LG_1_BIT_ICON_) || defined(_LG_2_BIT_ICON_)
    unsigned char     data    = 0;
    #endif

    unsigned char     index   = 0;
             int      x = ((GUI_RECT *)rect)->left;
             int      y = ((GUI_RECT *)rect)->top;


    SCREEN_COLOR      screen_color;

    GUI_COLOR         color;

    #ifdef  _LG_16_BIT_ICON_
    #ifdef  _LG_SCREEN_TO_SCREEN_COLOR_
    unsigned char     low_color  = 0;
    unsigned char     high_color = 0;
    #endif
    #endif

    #if  defined(_LG_24_BIT_ICON_) || defined(_LG_32_BIT_ICON_)
    unsigned char     red_color    = 0;
    unsigned char     green_color  = 0;
    unsigned char     blue_color   = 0;
    #endif

    #ifdef  _LG_32_BIT_ICON_
    unsigned char     alpha    = 0;
    #endif

    unsigned int      i       = 0;
    unsigned int      j       = 0;

             int      xfactor = 0;
             int      yfactor = 0;
                        
             int      sx      = 0;
             int      sy      = 0;
    unsigned char     tchar   = 0;
             int      ret     = 0;



    if ( pdata == NULL )
        return  -1;


    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + ((GUI_RECT *)rect)->left, hdc->rect.top + ((GUI_RECT *)rect)->top, \
                                 hdc->rect.left + ((GUI_RECT *)rect)->right, hdc->rect.top + ((GUI_RECT *)rect)->bottom );
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


    ret =  in_check_icon( (void *)pdata );
    if ( ret < 0 )
    {
        (lscrn.output_sequence_end)();
        return  -1;
    }

    raw_width     = pdata->width;
    raw_height    = pdata->height;
    line_char_num = pdata->bytes_per_line;
    and_char_num  = (raw_width+31)/32*4;

    real_width    = ((GUI_RECT *)rect)->right - ((GUI_RECT *)rect)->left;
    real_height   = ((GUI_RECT *)rect)->bottom - ((GUI_RECT *)rect)->top;

    /* scaled by 65536 */
    xfactor = muldiv64(raw_width,  65536, real_width);
    /* scaled by 65536 */
    yfactor = muldiv64(raw_height, 65536, real_height);


    switch ( pdata->bits_per_pixel )
    {
        /* palette color */
        #ifdef  _LG_1_BIT_ICON_
        case  1:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++ )
            {
                sx = 0;
                for ( i = 0; i < real_width; i++ )
                {
                    and_offset  = and_char_num * (raw_height-(sy>>16)-1) + (sx>>16)/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;


                    offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx>>16)/8;
                    if ( offset > (line_char_num * raw_height - 1) )
                        offset  = (line_char_num * raw_height - 1);

                    bit    = (i%8);

                    data   = *((pdata->xor_data)+offset);
                    data   = (data<<bit)&0x80;
                    if ( data == 0x80 )
                        index = 1;
                    else
                        index = 0;

                    color = pdata->palette->entries[index];
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    sx += xfactor;
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_1_BIT_ICON_ */

        #ifdef  _LG_2_BIT_ICON_
        case  2:
            break;
        #endif  /* _LG_2_BIT_ICON_ */

        #ifdef  _LG_4_BIT_ICON_
        case  4:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++ )
            {
                sx = 0;
                for ( i = 0; i < real_width; i++ )
                {
                    and_offset  = and_char_num * (raw_height-(sy>>16)-1) + (sx>>16)/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;

                    offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16)/2;
                    if ( offset > (line_char_num * raw_height - 1) )
                        offset  = (line_char_num * raw_height - 1);

                    bit    = (i%2);

                    index  = *((pdata->xor_data)+offset);
                    if ( bit > 0 )
                        index &= 0x0F;
                    else
                        index = (index>>4)&0x0F;

                    color  = pdata->palette->entries[index];
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    sx += xfactor;
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_4_BIT_ICON_ */

        #ifdef  _LG_8_BIT_ICON_
        case  8:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++ )
            {
                sx = 0;
                for ( i = 0; i < real_width; i++ )
                {
                    and_offset  = and_char_num * (raw_height-(sy>>16)-1) + (sx>>16)/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;

                    offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16);
                    if ( offset > (line_char_num * raw_height - 1) )
                        offset  = (line_char_num * raw_height - 1);

                    index  = *((pdata->xor_data)+offset);

                    color  = pdata->palette->entries[index];
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    sx += xfactor;
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_8_BIT_ICON_ */

        /* true color */
        #ifdef  _LG_16_BIT_ICON_
        #ifdef  _LG_SCREEN_TO_SCREEN_COLOR_
        case  16:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++ )
            {
                sx = 0;
                for ( i = 0; i < real_width; i++ )
                {
                    and_offset  = and_char_num * (raw_height-(sy>>16)-1) + (sx>>16)/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;


                    offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16)*2;
                    if ( offset > (line_char_num * raw_height - 2) )
                        offset  = (line_char_num * raw_height - 2);

                    low_color  = *((pdata->xor_data)+offset+0);
                    high_color = *((pdata->xor_data)+offset+1);

                    /* 16-bit screen color to  current screen color */
                    color  = (high_color << 8) | low_color;
                    in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    sx += xfactor;
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_SCREEN_TO_SCREEN_COLOR_ */
        #endif  /* _LG_16_BIT_ICON_ */

        #ifdef  _LG_24_BIT_ICON_
        case  24:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++)
            {
                sx = 0;
                for ( i = 0; i < real_width; i++ )
                {
                    and_offset  = and_char_num * (raw_height-(sy>>16)-1) + (sx>>16)/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;


                    offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16)*3;
                    if ( offset > (line_char_num * raw_height - 3) )
                        offset  = (line_char_num * raw_height - 3);

                    blue_color  = *((pdata->xor_data)+offset + 0);
                    green_color = *((pdata->xor_data)+offset + 1);
                    red_color   = *((pdata->xor_data)+offset + 2);

                    color = (red_color<<16)|(green_color<<8)|blue_color;
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    sx += xfactor;
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_24_BIT_ICON_ */

        #ifdef  _LG_32_BIT_ICON_
        case  32:
        {
            sy = 0;
            for ( j = 0; j < real_height; j++)
            {
                sx = 0;
                for ( i = 0; i < real_width; i++ )
                {
                    and_offset  = and_char_num * (raw_height-(sy>>16)-1) + (sx>>16)/8;
                    tchar = *(pdata->and_data+and_offset);
                    tchar = (tchar>>(7-(i%8)))&0x01;
                    if ( tchar )
                        continue;


                    offset      = line_char_num * (raw_height-(sy>>16)-1) + (sx >> 16)*4;
                    if ( offset > (line_char_num * raw_height - 4) )
                        offset  = (line_char_num * raw_height - 4);

                    blue_color  = *((pdata->xor_data)+offset + 0);
                    green_color = *((pdata->xor_data)+offset + 1);
                    red_color   = *((pdata->xor_data)+offset + 2);
                    alpha       = *((pdata->xor_data)+offset + 3);
                    if ( alpha == 0 )
                        continue;
 
                    color = (red_color<<16)|(green_color<<8)|blue_color;
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                    sx += xfactor;
                }
                sy += yfactor;
            }
        }
        break;
        #endif  /* _LG_32_BIT_ICON_ */

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
int  icon_fill_rect(HDC hdc, const void *rect, const void *icon )
{
    int  ret = 0;

    gui_lock();
    ret = in_icon_fill_rect(hdc, rect, icon);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_FILL_ICON_EXTENSION_ */

#endif    /* _LG_ICON_ */
