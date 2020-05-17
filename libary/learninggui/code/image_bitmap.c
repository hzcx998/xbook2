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
#include  <math.h>

#include  <lgmacro.h>

#include  <lock.h>
#include  <cursor.h>

#include  <d2_pixel.h>
#include  <screen.h>

#include  <image_comm.h>
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
			    if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }  
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
		            if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    } 
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
	                    if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    } 
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
			    if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
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
			    if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
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
			    if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
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




int  in_bitmap_rotate(HDC hdc, int x, int y, const void *bitmap, void *rotate)
{
    const GUI_BITMAP  *pdata = (GUI_BITMAP *)bitmap;
    unsigned int      raw_width  = 0;
    unsigned int      raw_height = 0;
    unsigned int      line_char_num = 0;
    unsigned int      offset  = 0;
             int      cur_x   = 0;
             int      cur_y   = 0;
             int      new_x   = 0;
	     int      new_y   = 0;
	     int      x0      = 0;
	     int      y0      = 0;
	     int      theta   = 0;
    GUI_POINT         plt     = {0, 0};
    GUI_POINT         plb     = {0, 0};
    GUI_POINT         prt     = {0, 0};
    GUI_POINT         prb     = {0, 0};
    GUI_RECT          rect    = {0, 0, 0, 0};
          double      fsin    = 0;
	  double      fcos    = 0;
          double      fvalue1 = 0;
          double      fvalue2 = 0;


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
             int      new_i   = 0;
	     int      new_j   = 0;
             int      ret     = 0;



    if ( pdata == NULL )
        return  -1;
    if ( rotate == NULL )
        return  -1;


    x0    = ((GUI_ROTATE *)rotate)->x;
    y0    = ((GUI_ROTATE *)rotate)->y;
    theta = ((GUI_ROTATE *)rotate)->theta;


    raw_width  = pdata->width;
    raw_height = pdata->height;


    fsin = sin((theta*PI)/IMAGE_ROTATE_180);
    fcos = cos((theta*PI)/IMAGE_ROTATE_180);


    /* LeftTop point */
    fvalue1 =  ((int)(x-x0))*fcos;
    fvalue2 = -((int)(y-y0))*fsin;
    plt.x   =  fvalue1 + fvalue2 + (int)x0;

    fvalue1 =  ((int)(x-x0))*fsin;
    fvalue2 =  ((int)(y-y0))*fcos;
    plt.y   =  fvalue1 + fvalue2 + (int)y0;

    /* LeftBottom point */
    fvalue1 =  ((int)(x-x0))*fcos;
    fvalue2 = -((int)(y+raw_height-1-y0))*fsin;
    plb.x   =  fvalue1 + fvalue2 + (int)x0;

    fvalue1 =  ((int)(x-x0))*fsin;
    fvalue2 =  ((int)(y+raw_height-1-y0))*fcos;
    plb.y   =  fvalue1 + fvalue2 + (int)y0;

    /* RightTop point */
    fvalue1 =  ((int)(x+raw_width-1-x0))*fcos;
    fvalue2 = -((int)(y-y0))*fsin; 
    prt.x   =  fvalue1 + fvalue2 + (int)x0;

    fvalue1 =  ((int)(x+raw_width-1-x0))*fsin;
    fvalue2 =  ((int)(y-y0))*fcos;
    prt.y   =  fvalue1 + fvalue2 + (int)y0;

    /* RightBottom point */
    fvalue1 =  ((int)(x+raw_width-1-x0)*fcos);
    fvalue2 = -((int)(y+raw_height-1-y0))*fsin;
    prb.x   =  (int)(fvalue1 + fvalue2 + x0);

    fvalue1 =  ((int)(x+raw_width-1-x0)*fsin);
    fvalue2 =  ((int)(y+raw_height-1-y0))*fcos;
    prb.y   =   fvalue1 + fvalue2 + (int)y0;


    /* Calculate rect */
    i = GUI_MIN(plt.x, plb.x);
    j = GUI_MIN(prt.x, prb.x);
    rect.left = GUI_MIN(i,j);


    i = GUI_MIN(plt.y, plb.y);
    j = GUI_MIN(prt.y, prb.y);
    rect.top = GUI_MIN(i,j);


    i = GUI_MAX(plt.x, plb.x);
    j = GUI_MAX(prt.x, prb.x);
    rect.right = GUI_MAX(i,j);

    i = GUI_MAX(plt.y, plb.y);
    j = GUI_MAX(prt.y, prb.y);
    rect.bottom = GUI_MAX(i,j);


    /* Adjust rect */
    if ( (rect.left) > (rect.right) )
    {
        i = rect.left;
	rect.left  = rect.right;
	rect.right = i;
    }

    if ( (rect.top) > (rect.bottom) )
    {
        i = rect.top;
	rect.top    = rect.bottom;
	rect.bottom = i;
    }


    hdc->is_paint_rect = 1;
    hdc->paint_rect = rect;


    /* New width and height */
    raw_width  = GUI_RECTW(&rect);
    raw_height = GUI_RECTH(&rect);




    /* Start to draw */
    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + rect.left, hdc->rect.top + rect.top, hdc->rect.right, hdc->rect.bottom);
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

    line_char_num = pdata->bytes_per_line;

    switch ( pdata->bits_per_pixel )
    {
        /* palette color */
        #ifdef  _LG_1_BIT_BITMAP_
        case  1:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = j + rect.top;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = i + rect.left;

                    /* Caliculate new position */
                    fvalue1 =  ((int)(cur_x-x0))*fcos;
                    fvalue2 =  ((int)(cur_y-y0))*fsin;
                    new_x   =  (int)(fvalue1 + fvalue2 + (int)x0);
                    new_i   =  new_x - x;
	            if ( (new_i <  0) || ( new_i > (pdata->width)-1) )
			continue;

                    fvalue1 = -((int)(cur_x-x0))*fsin;
                    fvalue2 =  ((int)(cur_y-y0))*fcos;
                    new_y   =  (int)(fvalue1 + fvalue2 + (int)y0);
		    new_j   = new_y - y;
                    if ( (new_j < 0) || ( new_j >(pdata->height) - 1) )
		        continue;


                     offset = line_char_num*(pdata->height-new_j-1) + new_i/8;
	             if ( offset > (pdata->height)*line_char_num - 1 )
	                continue;

                     bit    = (new_i%8);

                     data   = *((pdata->data)+offset);
                     data   = (data<<bit)&0x80;
                     if ( data == 0x80 )
                         index = 1;
                     else
                         index = 0;


                     color  = pdata->palette->entries[index];
		     if ( (pdata->is_transparent) > 0 ) 
	             { 
	                 if ( (pdata->transparent_color) == color )
		             continue;				 
	             }

                     screen_color = (lscrn.gui_to_screen_color)(color);
                     in_output_screen_pixel_abs(hdc, cur_x + hdc->rect.left,  cur_y + hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_1_BIT_BITMAP_ */

        #ifdef  _LG_2_BIT_BITMAP_
	/* ?? */
        case  2:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = j + rect.top;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = i + rect.left;

                    /* Caliculate new position */
                    fvalue1 =  ((int)(cur_x-x0))*fcos;
                    fvalue2 =  ((int)(cur_y-y0))*fsin;
                    new_x   =  (int)(fvalue1 + fvalue2 + (int)x0);
                    new_i   =  new_x - x;
	            if ( (new_i <  0) || ( new_i > (pdata->width)-1) )
			continue;

                    fvalue1 = -((int)(cur_x-x0))*fsin;
                    fvalue2 =  ((int)(cur_y-y0))*fcos;
                    new_y   =  (int)(fvalue1 + fvalue2 + (int)y0);
		    new_j   = new_y - y;
                    if ( (new_j < 0) || ( new_j >(pdata->height) - 1) )
		        continue;

                    offset = line_char_num*(pdata->height-new_j-1) + new_i/4;
	            if ( offset > (pdata->height)*line_char_num - 1 )
	                continue;


                     bit    = (new_i%4);

                     data   = *((pdata->data)+offset);
                     data   = (data<<bit)&0xC0;
                     if ( data == 0xC0 )
                         index = 1;
	             else if ( data == 0x80 )
                         index = 2;
                     else if ( data == 0x40 )
                         index = 3; 
                     else
                         index = 0;


                     color  = pdata->palette->entries[index];
		     if ( (pdata->is_transparent) > 0 ) 
	             { 
	                 if ( (pdata->transparent_color) == color )
		             continue;				 
	             }

                     screen_color = (lscrn.gui_to_screen_color)(color);
                     in_output_screen_pixel_abs(hdc, cur_x + hdc->rect.left,  cur_y + hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_2_BIT_BITMAP_ */

        #ifdef  _LG_4_BIT_BITMAP_
        case  4:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = j + rect.top;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = i + rect.left;

                    /* Caliculate new position */
                    fvalue1 =  ((int)(cur_x-x0))*fcos;
                    fvalue2 =  ((int)(cur_y-y0))*fsin;
                    new_x   =  (int)(fvalue1 + fvalue2 + (int)x0);
                    new_i   =  new_x - x;
	            if ( (new_i <  0) || ( new_i > (pdata->width)-1) )
			continue;

                    fvalue1 = -((int)(cur_x-x0))*fsin;
                    fvalue2 =  ((int)(cur_y-y0))*fcos;
                    new_y   =  (int)(fvalue1 + fvalue2 + (int)y0);
		    new_j   = new_y - y;
                    if ( (new_j < 0) || ( new_j >(pdata->height) - 1) )
		        continue;


                    offset = line_char_num*(pdata->height-new_j-1) + (new_i/2);
	            if ( offset > (pdata->height)*line_char_num - 1 )
	                continue;

                    bit    = (new_i%2);

                    index  = *((pdata->data)+offset);
                    if ( bit > 0 )
                        index &= 0x0F;
                    else
                        index = (index>>4)&0x0F;

	     
                     color  = pdata->palette->entries[index];
		     if ( (pdata->is_transparent) > 0 ) 
	             { 
	                 if ( (pdata->transparent_color) == color )
		             continue;				 
	             }

                     screen_color = (lscrn.gui_to_screen_color)(color);
                     in_output_screen_pixel_abs(hdc, cur_x + hdc->rect.left,  cur_y + hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_4_BIT_BITMAP_ */

        #ifdef  _LG_8_BIT_BITMAP_
        case  8:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = j + rect.top;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = i + rect.left;

                    /* Caliculate new position */
                    fvalue1 =  ((int)(cur_x-x0))*fcos;
                    fvalue2 =  ((int)(cur_y-y0))*fsin;
                    new_x   =  (int)(fvalue1 + fvalue2 + (int)x0);
                    new_i   =  new_x - x;
	            if ( (new_i <  0) || ( new_i > (pdata->width)-1) )
			continue;

                    fvalue1 = -((int)(cur_x-x0))*fsin;
                    fvalue2 =  ((int)(cur_y-y0))*fcos;
                    new_y   =  (int)(fvalue1 + fvalue2 + (int)y0);
		    new_j   = new_y - y;
                    if ( (new_j < 0) || ( new_j >(pdata->height) - 1) )
		        continue;


                     offset = line_char_num*(pdata->height-new_j-1) +new_i;
	             if ( offset > (pdata->height)*line_char_num - 1 )
	                continue;

	     
                     index  = *((pdata->data)+offset);
                     color  = pdata->palette->entries[index];
		     if ( (pdata->is_transparent) > 0 ) 
	             { 
	                 if ( (pdata->transparent_color) == color )
		             continue;				 
	             }

                     screen_color = (lscrn.gui_to_screen_color)(color);
                     in_output_screen_pixel_abs(hdc, cur_x + hdc->rect.left,  cur_y + hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_8_BIT_BITMAP_ */


        /* true color */
        #ifdef  _LG_16_BIT_BITMAP_
	/* ?? */
        case  16:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = j + rect.top;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = i + rect.left;

                    /* Caliculate new position */
                    fvalue1 =  ((int)(cur_x-x0))*fcos;
                    fvalue2 =  ((int)(cur_y-y0))*fsin;
                    new_x   =  (int)(fvalue1 + fvalue2 + (int)x0);
                    new_i   =  new_x - x;
	            if ( (new_i <  0) || ( new_i > (pdata->width)-1) )
			continue;

                    fvalue1 = -((int)(cur_x-x0))*fsin;
                    fvalue2 =  ((int)(cur_y-y0))*fcos;
                    new_y   =  (int)(fvalue1 + fvalue2 + (int)y0);
		    new_j   = new_y - y;
                    if ( (new_j < 0) || ( new_j >(pdata->height) - 1) )
		        continue;

                    offset      = line_char_num*(pdata->height-new_j-1) + 2*new_i;
                    if ( offset > (pdata->height)*line_char_num - 2 )
	                continue;


                    low_color  = *((pdata->data)+offset+0);
                    high_color = *((pdata->data)+offset+1);

                    color  = (high_color << 8) | low_color;
		    if ( (pdata->is_transparent) > 0 ) 
                    { 
	                if ( (pdata->transparent_color) == color )
		            continue;				 
	            } 

                     screen_color = (lscrn.gui_to_screen_color)(color);
                     in_output_screen_pixel_abs(hdc, cur_x + hdc->rect.left,  cur_y + hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_16_BIT_BITMAP_ */

        #ifdef  _LG_24_BIT_BITMAP_
        case  24:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = j + rect.top;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = i + rect.left;

                    /* Caliculate new position */
                    fvalue1 =  ((int)(cur_x-x0))*fcos;
                    fvalue2 =  ((int)(cur_y-y0))*fsin;
                    new_x   =  (int)(fvalue1 + fvalue2 + (int)x0);
                    new_i   =  new_x - x;
	            if ( (new_i <  0) || ( new_i > (pdata->width)-1) )
			continue;

                    fvalue1 = -((int)(cur_x-x0))*fsin;
                    fvalue2 =  ((int)(cur_y-y0))*fcos;
                    new_y   =  (int)(fvalue1 + fvalue2 + (int)y0);
		    new_j   = new_y - y;
                    if ( (new_j < 0) || ( new_j >(pdata->height) - 1) )
		        continue;

                    offset      = line_char_num*(pdata->height-new_j-1) + 3*new_i; 
                    if ( offset > (pdata->height)*line_char_num - 3 )
	                continue;

                     blue_color  = *((pdata->data)+offset + 0);
                     green_color = *((pdata->data)+offset + 1);
                     red_color   = *((pdata->data)+offset + 2);

                     color = (red_color<<16)|(green_color<<8)|blue_color;
	             if ( (pdata->is_transparent) > 0 ) 
	             { 
			 if ( (pdata->transparent_color) == color )
		             continue;				 
		     }
                     screen_color = (lscrn.gui_to_screen_color)(color);
                     in_output_screen_pixel_abs(hdc, cur_x + hdc->rect.left,  cur_y + hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_24_BIT_BITMAP_ */

        #ifdef  _LG_32_BIT_BITMAP_
	/* ?? */
        case  32:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = j + rect.top;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = i + rect.left;

                    /* Caliculate new position */
                    fvalue1 =  ((int)(cur_x-x0))*fcos;
                    fvalue2 =  ((int)(cur_y-y0))*fsin;
                    new_x   =  (int)(fvalue1 + fvalue2 + (int)x0);
                    new_i   =  new_x - x;
	            if ( (new_i <  0) || ( new_i > (pdata->width)-1) )
			continue;

                    fvalue1 = -((int)(cur_x-x0))*fsin;
                    fvalue2 =  ((int)(cur_y-y0))*fcos;
                    new_y   =  (int)(fvalue1 + fvalue2 + (int)y0);
		    new_j   = new_y - y;
                    if ( (new_j < 0) || ( new_j >(pdata->height) - 1) )
		        continue;

                    offset      = line_char_num*(pdata->height-new_j-1) + 4*new_i;
                    if ( offset > (pdata->height)*line_char_num - 4 )
	                continue;

                     blue_color  = *((pdata->data)+offset + 0);
                     green_color = *((pdata->data)+offset + 1);
                     red_color   = *((pdata->data)+offset + 2);
                     /* alpha    = *((pdata->data)+offset + 3); */

                     color = (red_color<<16)|(green_color<<8)|blue_color;
	             if ( (pdata->is_transparent) > 0 ) 
	             { 
			 if ( (pdata->transparent_color) == color )
		             continue;				 
		     }
                     screen_color = (lscrn.gui_to_screen_color)(color);
                     in_output_screen_pixel_abs(hdc, cur_x + hdc->rect.left,  cur_y + hdc->rect.top, screen_color);
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
int  bitmap_rotate(HDC hdc, int x, int y, const void *bitmap, void *rotate)
{
    int  ret = 0;

    gui_lock( );
    ret = in_bitmap_rotate(hdc, x, y, bitmap, rotate);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */
 


int  in_bitmap_rotate_special(HDC hdc, int x, int y, const void *bitmap, void *rotate)
{
    const GUI_BITMAP  *pdata = (GUI_BITMAP *)bitmap;
    unsigned int      raw_width;
    unsigned int      raw_height;
    unsigned int      line_char_num = 0;
    unsigned int      offset  = 0;
             int      cur_x   = 0;
             int      cur_y   = 0;
             int      new_x   = 0;
	     int      new_y   = 0;
	     int      x0      = 0;
	     int      y0      = 0;
	     int      theta   = 0;
          double      fsin    = 0;
	  double      fcos    = 0;
          double      fvalue1 = 0;
          double      fvalue2 = 0;




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
    if ( rotate == NULL )
        return  -1;


    x0    = ((GUI_ROTATE *)rotate)->x;
    y0    = ((GUI_ROTATE *)rotate)->y;
    theta = ((GUI_ROTATE *)rotate)->theta;


    fsin = sin((theta*PI)/IMAGE_ROTATE_180);
    fcos = cos((theta*PI)/IMAGE_ROTATE_180);



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

			    /* No rotate */
		            if ( (theta%IMAGE_ROTATE_360) != 0 )
			    {
	                        /* Caliculate new position */
                                fvalue1 =  ((int)(cur_x-x0))*fcos;
                                fvalue2 =  ((int)(cur_y-y0))*fsin;
                                new_x   =  fvalue1 - fvalue2 + (int)x0;

                                fvalue1 =  ((int)(cur_x-x0))*fsin;
                                fvalue2 =  ((int)(cur_y-y0))*fcos;
                                new_y   =  fvalue1 + fvalue2 + (int)y0;
			    }

                            offset = line_char_num*(raw_height-j-1) + (i/8);
                            bit    = (i%8);

                            data   = *((pdata->data)+offset);
                            data   = (data<<bit)&0x80;
                            if ( data == 0x80 )
                                index = 1;
                            else
                                index = 0;

                            color = pdata->palette->entries[index];
		            if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
                            screen_color= (lscrn.gui_to_screen_color)(color);
			    if ( (theta%IMAGE_ROTATE_360) == 0 )
                                in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
			    else
                                in_output_screen_pixel_abs(hdc, new_x, new_y, screen_color); 
                       }
                   }
                }
            }
            break;
        #endif  /* _LG_1_BIT_BITMAP_ */

        #ifdef  _LG_2_BIT_BITMAP_
	/* ?? */
        case  2:
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

			    /* No rotate */
		            if ( (theta%IMAGE_ROTATE_360) != 0 )
			    {
	                        /* Caliculate new position */
                                fvalue1 =  ((int)(cur_x-x0))*fcos;
                                fvalue2 =  ((int)(cur_y-y0))*fsin;
                                new_x   =  fvalue1 - fvalue2 + (int)x0;

                                fvalue1 =  ((int)(cur_x-x0))*fsin;
                                fvalue2 =  ((int)(cur_y-y0))*fcos;
                                new_y   =  fvalue1 + fvalue2 + (int)y0;
			    }

                            offset = line_char_num*(raw_height-j-1) + (i/4);
                            bit    = (i%4);

                            index  = *((pdata->data)+offset);
                            if ( bit > 0 )
                                index &= 0x0F;
                            else
                                index = (index>>2)&0x0F;

                            color  = pdata->palette->entries[index];
			    if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
                            screen_color = (lscrn.gui_to_screen_color)(color);
			    if ( (theta%IMAGE_ROTATE_360) == 0 )
                                in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
			    else
                                in_output_screen_pixel_abs(hdc, new_x, new_y, screen_color); 
                       }
                   }
                }
            }
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

			    /* No rotate */
		            if ( (theta%IMAGE_ROTATE_360) != 0 )
			    {
	                        /* Caliculate new position */
                                fvalue1 =  ((int)(cur_x-x0))*fcos;
                                fvalue2 =  ((int)(cur_y-y0))*fsin;
                                new_x   =  fvalue1 - fvalue2 + (int)x0;

                                fvalue1 =  ((int)(cur_x-x0))*fsin;
                                fvalue2 =  ((int)(cur_y-y0))*fcos;
                                new_y   =  fvalue1 + fvalue2 + (int)y0;
			    }

                            offset = line_char_num*(raw_height-j-1) + (i/2);
                            bit    = (i%2);

                            index  = *((pdata->data)+offset);
                            if ( bit > 0 )
                                index &= 0x0F;
                            else
                                index = (index>>4)&0x0F;

                            color  = pdata->palette->entries[index];
		            if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    } 
                            screen_color = (lscrn.gui_to_screen_color)(color);
			    if ( (theta%IMAGE_ROTATE_360) == 0 )
                                in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
			    else
                                in_output_screen_pixel_abs(hdc, new_x, new_y, screen_color); 
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

			    /* No rotate */
		            if ( (theta%IMAGE_ROTATE_360) != 0 )
			    {
	                        /* Caliculate new position */
                                fvalue1 =  ((int)(cur_x-x0))*fcos;
                                fvalue2 =  ((int)(cur_y-y0))*fsin;
                                new_x   =  fvalue1 - fvalue2 + (int)x0;

                                fvalue1 =  ((int)(cur_x-x0))*fsin;
                                fvalue2 =  ((int)(cur_y-y0))*fcos;
                                new_y   =  fvalue1 + fvalue2 + (int)y0;
			    }

                            offset = line_char_num*(raw_height-j-1) + i;
                            index  = *((pdata->data)+offset);

                            color  = pdata->palette->entries[index];
		            if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    } 
                            screen_color = (lscrn.gui_to_screen_color)(color);
			    if ( (theta%IMAGE_ROTATE_360) == 0 )
                                in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
			    else
                                in_output_screen_pixel_abs(hdc, new_x, new_y, screen_color); 
                        }
                    }
                }
            }
            break;
        #endif  /* _LG_8_BIT_BITMAP_ */


        /* true color */
        #ifdef  _LG_16_BIT_BITMAP_
	/* ?? */
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

			    /* No rotate */
		            if ( (theta%IMAGE_ROTATE_360) != 0 )
			    {
	                        /* Caliculate new position */
                                fvalue1 =  ((int)(cur_x-x0))*fcos;
                                fvalue2 =  ((int)(cur_y-y0))*fsin;
                                new_x   =  fvalue1 - fvalue2 + (int)x0;

                                fvalue1 =  ((int)(cur_x-x0))*fsin;
                                fvalue2 =  ((int)(cur_y-y0))*fcos;
                                new_y   =  fvalue1 + fvalue2 + (int)y0;
			    }

                            offset     = line_char_num*(raw_height-j-1) + 2*i;
                            low_color  = *((pdata->data)+offset+0);
                            high_color = *((pdata->data)+offset+1);

                            /* 16-bit screen color to  current screen color */
                            color  = (high_color << 8) | low_color;
		            if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    } 
                            screen_color = (lscrn.gui_to_screen_color)(color);
			    if ( (theta%IMAGE_ROTATE_360) == 0 )
                                in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
			    else
                                in_output_screen_pixel_abs(hdc, new_x, new_y, screen_color); 
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

			    /* No rotate */
		            if ( (theta%IMAGE_ROTATE_360) != 0 )
			    {
	                        /* Caliculate new position */
                                fvalue1 =  ((int)(cur_x-x0))*fcos;
                                fvalue2 =  ((int)(cur_y-y0))*fsin;
                                new_x   =  fvalue1 - fvalue2 + (int)x0;

                                fvalue1 =  ((int)(cur_x-x0))*fsin;
                                fvalue2 =  ((int)(cur_y-y0))*fcos;
                                new_y   =  fvalue1 + fvalue2 + (int)y0;
			    }

                            offset      = line_char_num*(raw_height-j-1) + 3*i;

                            blue_color  = *((pdata->data)+offset + 0);
                            green_color = *((pdata->data)+offset + 1);
                            red_color   = *((pdata->data)+offset + 2);

                            color = (red_color<<16)|(green_color<<8)|blue_color;
		            if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
                            screen_color = (lscrn.gui_to_screen_color)(color);
			    if ( (theta%IMAGE_ROTATE_360) == 0 )
                                in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
			    else
                                in_output_screen_pixel_abs(hdc, new_x, new_y, screen_color); 
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

			    /* No rotate */
		            if ( (theta%IMAGE_ROTATE_360) != 0 )
			    {
	                        /* Caliculate new position */
                                fvalue1 =  ((int)(cur_x-x0))*fcos;
                                fvalue2 =  ((int)(cur_y-y0))*fsin;
                                new_x   =  fvalue1 - fvalue2 + (int)x0;

                                fvalue1 =  ((int)(cur_x-x0))*fsin;
                                fvalue2 =  ((int)(cur_y-y0))*fcos;
                                new_y   =  fvalue1 + fvalue2 + (int)y0;
			    }

                            offset      = line_char_num*(raw_height-j-1) + 4*i;

                            blue_color  = *((pdata->data)+offset + 0);
                            green_color = *((pdata->data)+offset + 1);
                            red_color   = *((pdata->data)+offset + 2);
                            /* alpha    = *((pdata->data)+offset + 3); */

                            /* alpha ?? */
                            color = (red_color<<16)|(green_color<<8)|blue_color;
		            if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
                            screen_color = (lscrn.gui_to_screen_color)(color);
			    if ( (theta%IMAGE_ROTATE_360) == 0 )
                                in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
			    else
                                in_output_screen_pixel_abs(hdc, new_x, new_y, screen_color);
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
int  bitmap_rotate_special(HDC hdc, int x, int y, const void *bitmap, void *rotate)
{
    int  ret = 0;

    gui_lock( );
    ret = in_bitmap_rotate_special(hdc, x, y, bitmap, rotate);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */
 


int  in_bitmap_symmetry_special(HDC hdc, int x, int y, const void *bitmap, void *symmetry)
{
    const GUI_BITMAP  *pdata = (GUI_BITMAP *)bitmap;
    GUI_SYMMETRY      *psymmetry = (GUI_SYMMETRY *)symmetry;
    unsigned int       raw_width;
    unsigned int       raw_height;
    unsigned int       line_char_num = 0;
    unsigned int       offset  = 0;
             int       cur_x = 0;
             int       cur_y = 0;
             int       new_x = 0;
	     int       new_y = 0;


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
    if ( psymmetry == NULL )
        return  -1;

    if ( (psymmetry->is_symmetry) && ((psymmetry->symmetry_type) == GUI_SYMMETRY_LINE) )
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
                cur_y = y+j;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = x + i;

                    offset = line_char_num*(raw_height-j-1) + (i/8);
                    bit    = (i%8);

                    data   = *((pdata->data)+offset);
                    data   = (data<<bit)&0x80;
                    if ( data == 0x80 )
                        index = 1;
                    else
                        index = 0;

                    color = pdata->palette->entries[index];
	            if ( (pdata->is_transparent) > 0 ) 
	            { 
			if ( (pdata->transparent_color) == color )
		            continue;				 
		    }  
                    screen_color= (lscrn.gui_to_screen_color)(color);

	            if ( (psymmetry->is_symmetry) == 0 )
		    {
                        in_output_screen_pixel_abs(hdc, cur_x+hdc->rect.left, cur_y+hdc->rect.top, screen_color);
		        continue;
		    }

		    if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (psymmetry->point[0].x)*2 - cur_x;
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
	            }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
			new_x = cur_x;
		    }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_LINE ) {
		        continue;
		    } else {
	                new_x = (psymmetry->point[0].x)*2 - cur_x;
			new_y = cur_y;
		    } 
                    in_output_screen_pixel_abs(hdc, new_x+hdc->rect.left, new_y+hdc->rect.top, screen_color);
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
                cur_y = y+j;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = x + i;

                    offset = line_char_num*(raw_height-j-1) + (i/2);
                    bit    = (i%2);

                    index  = *((pdata->data)+offset);
                    if ( bit > 0 )
                        index &= 0x0F;
                    else
                        index = (index>>4)&0x0F;

                    color  = pdata->palette->entries[index];
		    if ( (pdata->is_transparent) > 0 ) 
	            { 
			if ( (pdata->transparent_color) == color )
			    continue;				 
		    } 
                    screen_color = (lscrn.gui_to_screen_color)(color);

		    if ( (psymmetry->is_symmetry) == 0 )
		    {
                        in_output_screen_pixel_abs(hdc, cur_x+hdc->rect.left, cur_y+hdc->rect.top, screen_color);
			continue;
		    }

		    if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (psymmetry->point[0].x)*2 - cur_x;
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
		    }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
			new_x = cur_x;
		    }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_LINE ) {
		        continue;
		    } else {
	                new_x = (psymmetry->point[0].x)*2 - cur_x;
			new_y = cur_y;
		    } 
                    in_output_screen_pixel_abs(hdc, new_x+hdc->rect.left, new_y+hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_4_BIT_BITMAP_ */

        #ifdef  _LG_8_BIT_BITMAP_
        case  8:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = y+j;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = x + i;

                    offset = line_char_num*(raw_height-j-1) + i;
                    index  = *((pdata->data)+offset);

                    color  = pdata->palette->entries[index];
	            if ( (pdata->is_transparent) > 0 ) 
	            { 
			if ( (pdata->transparent_color) == color )
		            continue;				 
		    } 
                    screen_color = (lscrn.gui_to_screen_color)(color);

		    if ( (psymmetry->is_symmetry) == 0 )
		    {
                        in_output_screen_pixel_abs(hdc, cur_x+hdc->rect.left, cur_y+hdc->rect.top, screen_color);
		        continue;
		    }

		    if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (psymmetry->point[0].x)*2 - cur_x;
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
		    }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
			new_x = cur_x;
		    }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_LINE ) {
			continue;
		    } else {
	                new_x = (psymmetry->point[0].x)*2 - cur_x;
			new_y = cur_y;
		    } 
                    in_output_screen_pixel_abs(hdc, new_x+hdc->rect.left, new_y+hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_8_BIT_BITMAP_ */


        /* true color */
        #ifdef  _LG_16_BIT_BITMAP_
        case  16:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = y+j;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = x + i;
                    offset     = line_char_num*(raw_height-j-1) + 2*i;
                    low_color  = *((pdata->data)+offset+0);
                    high_color = *((pdata->data)+offset+1);

                    /* 16-bit screen color to  current screen color */
                    color  = (high_color << 8) | low_color; 
	            if ( (pdata->is_transparent) > 0 ) 
		    { 
			if ( (pdata->transparent_color) == color )
			    continue;				 
		    }
                    screen_color = (lscrn.gui_to_screen_color)(color);

	            if ( (psymmetry->is_symmetry) == 0 )
		    {
                        in_output_screen_pixel_abs(hdc, cur_x+hdc->rect.left, cur_y+hdc->rect.top, screen_color);
		        continue;
		    }

		    if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (psymmetry->point[0].x)*2 - cur_x;
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
		    }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
			new_x = cur_x;
		    }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_LINE ) {
			continue;
		    } else {
	                new_x = (psymmetry->point[0].x)*2 - cur_x;
			new_y = cur_y;
		    } 
                    in_output_screen_pixel_abs(hdc, new_x+hdc->rect.left, new_y+hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_16_BIT_BITMAP_ */

        #ifdef  _LG_24_BIT_BITMAP_
        case  24:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = y+j;
                for ( i = 0; i < raw_width; i++ )
                {
                    cur_x = x + i;

                    offset      = line_char_num*(raw_height-j-1) + 3*i;

                    blue_color  = *((pdata->data)+offset + 0);
                    green_color = *((pdata->data)+offset + 1);
                    red_color   = *((pdata->data)+offset + 2);

                    color = (red_color<<16)|(green_color<<8)|blue_color;
	            if ( (pdata->is_transparent) > 0 ) 
		    { 
			if ( (pdata->transparent_color) == color )
		            continue;				 
		    }
                    screen_color = (lscrn.gui_to_screen_color)(color);


	            if ( (psymmetry->is_symmetry) == 0 )
		    {
                         in_output_screen_pixel_abs(hdc, cur_x + hdc->rect.left, cur_y + hdc->rect.top, screen_color);
		         continue;
		    }

		    if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (psymmetry->point[0].x)*2 - cur_x;
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
		    }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
			new_x = cur_x;
		    }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_LINE ) {
			continue;
		    } else {
	                new_x = (psymmetry->point[0].x)*2 - cur_x;
			new_y = cur_y;
	            } 
                    in_output_screen_pixel_abs(hdc, new_x + hdc->rect.left, new_y + hdc->rect.top, screen_color);
                }
            }
            break;
        #endif  /* _LG_24_BIT_BITMAP_ */

        #ifdef  _LG_32_BIT_BITMAP_
        case  32:
            for ( j = 0; j < raw_height; j++ )
            {
                cur_y = y+j;
                for ( i = 0; i < raw_width; i++ )
                {                     
                    cur_x = x + i;

                    offset      = line_char_num*(raw_height-j-1) + 4*i;

                    blue_color  = *((pdata->data)+offset + 0);
                    green_color = *((pdata->data)+offset + 1);
                    red_color   = *((pdata->data)+offset + 2);
                    /*
                    alpha       = *((pdata->data)+offset + 3);
                    */

                    /* alpha ?? */
                    color = (red_color<<16)|(green_color<<8)|blue_color;
	            if ( (pdata->is_transparent) > 0 ) 
		    { 
			if ( (pdata->transparent_color) == color )
			    continue;				 
		    }
                    screen_color = (lscrn.gui_to_screen_color)(color);

	            if ( (psymmetry->is_symmetry) == 0 )
		    {
                        in_output_screen_pixel_abs(hdc, cur_x + hdc->rect.left, cur_y + hdc->rect.top, screen_color);
		        continue;
		    }

		    if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (psymmetry->point[0].x)*2 - cur_x;
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
	            }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (psymmetry->point[0].y)*2 - cur_y;
			new_x = cur_x;
	            }  else if ( (psymmetry->symmetry_type) == GUI_SYMMETRY_LINE ) {
	        	continue;
		    } else {
	                new_x = (psymmetry->point[0].x)*2 - cur_x;
			new_y = cur_y;
	            } 

                    in_output_screen_pixel_abs(hdc, new_x + hdc->rect.left, new_y + hdc->rect.top, screen_color);
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
int  bitmap_symmetry_special(HDC hdc, int x, int y, const void *bitmap, void *symmetry)
{
    int  ret = 0;

    gui_lock( );
    ret = in_bitmap_symmetry_special(hdc, x, y, bitmap, symmetry);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */





#ifdef   _LG_FILL_BITMAP_EXTENSION_

#define  muldiv64(m1, m2, d)          (((long long int)(m1*m2))/d) 

/* This is a DDA-based algorithm. */
int  in_bitmap_scale(HDC hdc, const void *rect, const void *bitmap)
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
    if ( p ==  NULL )
	return  -1;


    if ( (p->left) > (p->right) )
        return  -1;
    if ( (p->top) > (p->bottom) )
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

    if ( raw_width < 1 )
        return  -1;
    if ( raw_height < 1 )
        return  -1;


    real_width    = p->right - p->left;
    real_height   = p->bottom - p->top;

    if ( real_width < 1 )
        return  -1;
    if ( real_height < 1 )
        return  -1;

   
    /* scaled by 65536 */
    xfactor = muldiv64(raw_width,  65536, real_width); 
    /* scaled by 65536 */
    yfactor = muldiv64(raw_height, 65536, real_height);        


    switch ( pdata->bits_per_pixel )
    {
        /* palette color */
        #ifdef  _LG_1_BIT_BITMAP_
        case  1:
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
			    /*
		            if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
			    */
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
            break;
        #endif  /* _LG_1_BIT_BITMAP_ */

        #ifdef  _LG_2_BIT_BITMAP_
        case  2:
            break;
        #endif  /* _LG_2_BIT_BITMAP_ */

        #ifdef  _LG_4_BIT_BITMAP_
        case  4:
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
			    /*
		            if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
			    */
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
            break;
        #endif  /* _LG_4_BIT_BITMAP_ */

        #ifdef  _LG_8_BIT_BITMAP_
        case  8:
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
			    /*
			    if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
			    */
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
            break;
        #endif  /* _LG_8_BIT_BITMAP_ */


        /* true color */
        #ifdef  _LG_16_BIT_BITMAP_
        #ifdef  _LG_SCREEN_TO_SCREEN_COLOR_
        case  16:
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
			    /*
			    if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    }
			    */
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            }
            break;
        #endif  /* _LG_SCREEN_TO_SCREEN_COLOR_ */
        #endif  /* _LG_16_BIT_BITMAP_ */

        #ifdef  _LG_24_BIT_BITMAP_
        case  24:
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
			    /*
			    if ( (pdata->is_transparent) > 0 ) 
			    { 
			        if ( (pdata->transparent_color) == color )
				    continue;				 
			    } 
			    */
                            screen_color = (lscrn.gui_to_screen_color)(color);
                            in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
            } 
            break;
        #endif  /* _LG_24_BIT_BITMAP_ */

        #ifdef  _LG_32_BIT_BITMAP_
        case  32:
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
				/*
				if ( (pdata->is_transparent) > 0 ) 
			        {    
			            if ( (pdata->transparent_color) == color )
				        continue;				 
			        }
				*/
                                screen_color = (lscrn.gui_to_screen_color)(color);
                                in_output_screen_pixel_abs(hdc, cur_x, cur_y, screen_color);
                            }
                        }
                        sx += xfactor;
                    }
                }
                sy += yfactor;
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
int  bitmap_scale(HDC hdc, const void *rect, const void *bitmap)
{
    int  ret = 0;

    gui_lock( );
    ret = in_bitmap_scale(hdc, rect, bitmap);
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
