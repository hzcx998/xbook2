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

#include  <file_ops.h>


#ifdef  _LG_FILE_SYSTEM_


/* Tmp path */
/*
static  unsigned char  lspath[FILENAME_MAX] = "";
*/


/*
int  stream_create_path(const char *path)
{
    int   len = 0;
    int   i   = 0;
    int   ret = 0;


    memset(lspath, 0, sizeof(lspath));
    strcpy((char *)lspath, path);
    len = strlen((const char *)lspath);

    for ( i = 1; i < len; i++ )
    {
        if ( lspath[i] != '/' )
            continue;

        lspath[i] = '\0';
        ret = access((const char *)lspath, F_OK);
        if ( ret < 0 )
        {
            if ( mkdir((const char *)lspath, S_IRUSR | S_IRGRP | S_IROTH ) < 0 )
                return  -1;
        }
        lspath[i] = '/';
    }

    return  1;
}
*/

FILE  *stream_open_file(const char *path, const char *mode)
{
    if ( path == NULL )
        return  NULL;

    /*
    ret = stream_create_path(path);
    if ( ret < 0 )
        return  -1;
    */

    return  fopen(path, mode);
}
 
int  stream_read_file(FILE *stream, void *buf, size_t len)
{
    size_t  num = 0;


    if ( stream == NULL )
        return  -1;
    if ( buf == NULL )
        return  -1;

    num = fread(buf, len, 1, stream);

    return  num;
}

int  stream_write_file(FILE *stream, const void *buf, size_t len)
{
    size_t  num = 0;


    if ( stream == NULL )
        return  -1;
    if ( buf == NULL )
        return  -1;

    num = fwrite(buf, len, 1, stream);

    return  num;
}

int  stream_close_file(FILE *stream)
{
    fclose(stream);

    return  1;
}
    
int  stream_remove_file(const char *path)
{
    if ( path == NULL )
        return  -1;

    remove(path);

    return  1;
}

#endif  /* _LG_FILE_SYSTEM_ */
