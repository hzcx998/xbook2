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

/* Image type */
#define  IMAGE_BITMAP               (1<<0)
#define  IMAGE_ICON                 (1<<1)
#define  IMAGE_GIF                  (1<<2)

/* Image align format */
#define  IMAGE_ALIGN_LEFT           (1<<0)
#define  IMAGE_ALIGN_TOP            (1<<1)
#define  IMAGE_ALIGN_RIGHT          (1<<2)
#define  IMAGE_ALIGN_BOTTOM         (1<<3)
#define  IMAGE_ALIGN_HCENTER        (1<<4)
#define  IMAGE_ALIGN_VCENTER        (1<<5)
#define  IMAGE_ALIGN_CENTER         (IMAGE_ALIGN_HCENTER | IAMGE_ALIGN_VCENTER)
#define  IMAGE_ALIGN_FILL           (1<<6)


#ifdef  __cplusplus
extern  "C"
{
#endif


    /*
    #if  defined(_LG_BITMAP_) || defined(_LG_ICON_) || defined(_LG_GIF_)
    */

    int  in_image_decode_malloc_buffer(unsigned int size); 
    int  in_image_decode_remalloc_buffer(unsigned int size); 
    int  in_image_decode_free_buffer(void);


    #ifndef  _LG_ALONE_VERSION_
    int  image_decode_malloc_buffer(unsigned int size); 
    int  image_decode_remalloc_buffer(unsigned int size); 
    int  image_decode_free_buffer(void);
    #else
    #define  image_decode_malloc_buffer(size)              in_image_decode_malloc_buffer(size)
    #define  image_decode_remalloc_buffer(size)            in_image_decode_remalloc_buffer(size)
    #define  image_decode_free_buffer()                    in_image_decode_free_buffer()
    #endif

    /* #endif */ /* defined(_LG_BITMAP_) || defined(_LG_ICON_) || defined(_LG_GIF_) */


#ifdef  __cplusplus
}
#endif

#endif  /* __LGUI_IMAGE_COMMON_HEADER__ */
