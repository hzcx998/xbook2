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
#include  <stdlib.h>
#include  <math.h>

#include  <type_color.h>
#include  <type_gui.h>

#include  <lgmacro.h>
#include  <lock.h>

#include  <image_bitmap.h>



#define   IMAGE_DECODE_MIN_SIZE          32


static  volatile  unsigned char  *ldatap = NULL;

    

int  in_image_decode_malloc_buffer(unsigned int size)
{
    if ( size < IMAGE_DECODE_MIN_SIZE )
        size = IMAGE_DECODE_MIN_SIZE;


    if ( ldatap != NULL )
        return  -1;


    ldatap = malloc(size);
    if (ldatap == NULL )
        return  -1;


    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  image_decode_malloc_buffer(unsigned int size)
{
    int  ret = 0;

    gui_lock( );
    ret = in_image_decode_malloc_buffer(size);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_image_decode_remalloc_buffer(unsigned int size)
{
    int  ret = 0;


    if ( ldatap != NULL )
    {
        free(((void *)ldatap));
        ldatap = NULL;
    }

    ret = in_image_decode_malloc_buffer(size);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  image_decode_remalloc_buffer(unsigned int size)
{
    int  ret = 0;

    gui_lock( );
    ret = in_image_decode_remalloc_buffer(size);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_image_decode_free_buffer(void)
{
    if ( ldatap == NULL )
        return  0;

    free(((void *)ldatap));

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  image_decode_free_buffer(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_image_decode_free_buffer();
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */



int  in_gui_round(double x)
{
    return  (int)(x + 0.5);
}

int  in_gui_round_up(double x)
{
    if ( GUI_ABS(x - (int)(x + 5e-10)) < 1e-9 ) 
	return (int)(x + 5e-10); 
    else 
	return (int)(x + 1); 
}


unsigned char  in_gui_byte(double x)
{
    int  y = 0;

    y = in_gui_round(x);

    if ( y <= 0 )
        return  0x00;
    else if ( y >= 0xFF)
	return  0xFF;
    else
	return (unsigned char)y;
}



double  in_gui_cos(double angle)
{
    double value   = 0.0;
    double off     = 0.0;
    int    iangle  = 0;


    off = (angle/30 - in_gui_round(angle/30));
    if ( (off < 0.0000001) && (off > -0.0000001))
    {
        iangle = (int)in_gui_round(angle);
        iangle = (iangle < 0) ? (360 - (-iangle % 360))  : (iangle % 360);
        switch (iangle)
        {
            case 0: 
		value = 1.0; 
		break;

            case 30: 
		value = 0.866025403784439; 
		break;

            case 60: 
		value = 0.5; 
		break;

            case 90: 
		value = 0.0; 
		break;

            case 120: 
		value = -0.5; 
		break;

            case 150: 
		value = -0.866025403784439;
		break;

            case 180: 
		value = -1.0; 
		break;

            case 210: 
		value = -0.866025403784439; 
		break;

            case 240: 
		value = -0.5; 
		break;

            case 270: 
	        value = 0.0; 
		break;

            case 300: 
		value = 0.5; 
		break;

            case 330: 
		value = 0.866025403784439; 
		break;

            case 360: 
		value = 1.0; 
		break;

            default: 
		value = cos((angle * PI)/180);
        }

        return  value;

    } else {

        value =  cos(angle * PI/180);
        return  value;

    }
}

double  in_gui_sin(double angle)
{
    return  in_gui_cos(angle + 90.0);
}



int  in_gui_bitmap_get_color(const void *bitmap, unsigned int offset, unsigned char bit, char *transparent, GUI_COLOR *color)
{ 
    const GUI_BITMAP  *pbitmap = (GUI_BITMAP *)bitmap;

    GUI_COLOR       gui_color    = GUI_BLACK;

    unsigned char   red_color    = 0;
    unsigned char   green_color  = 0;
    unsigned char   blue_color   = 0;

    unsigned char   data    = 0;
    unsigned char   index   = 0;



    if ( pbitmap == NULL )
        return  -1;


    switch ( pbitmap->bits_per_pixel )
    {
        case  1:
            data   = *((pbitmap->data)+offset);
            data   = (data<<bit)&0x80;
            if ( data == 0x80 )
                index = 1;
            else
                index = 0;

            gui_color = pbitmap->palette->entries[index]; 
	    break;

	    /* ?? */
	case  2:
            data   = *((pbitmap->data)+offset);
            data   = (data<<bit)&0xFF;
            if ( data == 0x80 )
                index = 3;
            if ( data == 0x40 )
                index = 2;
	    if ( data == 0x20 )
                index = 1;
            else
                index = 0;

            gui_color  = pbitmap->palette->entries[index];
            break;

	case  4:
            index  = *((pbitmap->data)+offset);
            if ( bit > 0 )
                index &= 0x0F;
            else
                index = (index>>4)&0x0F;

            gui_color  = pbitmap->palette->entries[index];
            break;


        case  8:
            index      = *((pbitmap->data)+offset);
            gui_color  = pbitmap->palette->entries[index];
            break;

        case  24:
	    offset *= 3;
            if ( offset > ((pbitmap->height)*(pbitmap->bytes_per_line)- 3) )
	        return  -1;

            blue_color  = *((pbitmap->data)+offset + 0);
            green_color = *((pbitmap->data)+offset + 1);
            red_color   = *((pbitmap->data)+offset + 2);

            gui_color = (red_color<<16)|(green_color<<8)|blue_color;
	    break;

        case  32:
	    offset *= 4;
            if ( offset > ((pbitmap->height)*(pbitmap->bytes_per_line)- 4) )
	        return  -1;

            blue_color  = *((pbitmap->data)+offset + 0);
            green_color = *((pbitmap->data)+offset + 1);
            red_color   = *((pbitmap->data)+offset + 2);

            gui_color = (red_color<<16)|(green_color<<8)|blue_color;
            break;

        default:
            return  -1;

    }

    if ( (pbitmap->is_transparent) ==  0 ) 
    {
        *transparent = 0;
        *color       = gui_color;
	return  1;
    }

    if ( (pbitmap->transparent_color) == gui_color )
    {
        *transparent = 1;
        *color       = gui_color;
	return  1;
    }

    *transparent = 0;
    *color       = gui_color; 

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  gui_bitmap_get_color(const void *bitmap, unsigned int offset, unsigned char bit, char *transparent, GUI_COLOR *color)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_bitmap_get_color(bitmap, offset, bit, transparent, color);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */




