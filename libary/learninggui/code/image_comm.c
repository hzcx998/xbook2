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

#include  <lock.h>



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

