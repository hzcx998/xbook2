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

#ifndef  __LGUI_IMAGE_COMMON_HEADER__
#define  __LGUI_IMAGE_COMMON_HEADER__

#include  <type_gui.h>


/* Image type */
#define  IMAGE_BITMAP                   (1<<0)
#define  IMAGE_ICON                     (1<<1)
#define  IMAGE_GIF                      (1<<2)

/* Image align format */
#define  IMAGE_ALIGN_LEFT               (1<<0)
#define  IMAGE_ALIGN_TOP                (1<<1)
#define  IMAGE_ALIGN_RIGHT              (1<<2)
#define  IMAGE_ALIGN_BOTTOM             (1<<3)
#define  IMAGE_ALIGN_HCENTER            (1<<4)
#define  IMAGE_ALIGN_VCENTER            (1<<5)
#define  IMAGE_ALIGN_CENTER             (IMAGE_ALIGN_HCENTER | IAMGE_ALIGN_VCENTER)
#define  IMAGE_ALIGN_FILL               (1<<6)

/* Image rotate factor */
#define  IMAGE_ROTATE_FACTOR            100

/* Image rotate angle */
#define  IMAGE_ROTATE_0                 (  0*(IMAGE_ROTATE_FACTOR))
#define  IMAGE_ROTATE_90                ( 90*(IMAGE_ROTATE_FACTOR))
#define  IMAGE_ROTATE_180               (180*(IMAGE_ROTATE_FACTOR))
#define  IMAGE_ROTATE_270               (270*(IMAGE_ROTATE_FACTOR))
#define  IMAGE_ROTATE_360               (360*(IMAGE_ROTATE_FACTOR))


/* GUI_ROTATE_ANGLE macro */
#define  GUI_ROTATE_ANGLE(angle)        ((angle)*(IMAGE_ROTATE_FACTOR))



struct tagINDEX_LIST 
{
    struct tagINDEX_LIST  *next;
    int    index;
    int    used;
};
typedef  struct  tagINDEX_LIST  INDEX_LIST; 




#ifdef  __cplusplus
extern  "C"
{
#endif


    int  in_image_decode_malloc_buffer(unsigned int size); 
    int  in_image_decode_remalloc_buffer(unsigned int size); 
    int  in_image_decode_free_buffer(void);

    int  in_gui_bitmap_get_color(const void *bitmap, unsigned int offset, unsigned char bit, char *transparent,GUI_COLOR *color);


    #ifndef  _LG_ALONE_VERSION_
    int  image_decode_malloc_buffer(unsigned int size); 
    int  image_decode_remalloc_buffer(unsigned int size); 
    int  image_decode_free_buffer(void);
    int  gui_bitmap_get_color(const void *bitmap, unsigned int offset, unsigned char bit, char *transparent,GUI_COLOR *color);
    #else
    #define  image_decode_malloc_buffer(size)              in_image_decode_malloc_buffer(size)
    #define  image_decode_remalloc_buffer(size)            in_image_decode_remalloc_buffer(size)
    #define  image_decode_free_buffer()                    in_image_decode_free_buffer()

    #define  gui_bitmap_get_color(bitamp,offset,bit,transparent,color)   in_gui_bitmap_get_color(bitmap,offset,bit,transparent,color)
    #endif


    int  in_gui_round(double x); 
    #define  gui_round(x)     in_gui_round(x)

    int  in_gui_round_up(double x); 
    #define  gui_round_up(x)     in_gui_round_up(x)


    unsigned char  in_gui_byte(double x); 
    #define  gui_byte(x)     in_gui_byte(x)


    double  in_gui_sin(double x); 
    #define  gui_sin(x)     in_gui_sin(x)

    double  in_gui_cos(double x); 
    #define  gui_cos(x)     in_gui_cos(x)



#ifdef  __cplusplus
}
#endif

#endif  /* __LGUI_IMAGE_COMMON_HEADER__ */
