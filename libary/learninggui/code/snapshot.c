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
#include  <string.h>

#include  <lock.h>

#include  <screen.h>
#include  <image_bitmap.h>
#include  <file_ops.h>

#include  <snapshot.h>

#ifdef  _LG_SCREEN_
#ifdef  _LG_SNAPSHOT_
#ifdef  _LG_FILE_SYSTEM_

#ifdef  _LG_WINDOW_
#include  <win_dc.h>
#endif


#ifndef  SNAPSHOT_FILENAME_MAX
#define  SNAPSHOT_FILENAME_MAX                      62
#endif


/* Snapshot rect */
static  volatile  GUI_RECT      lshotr = {0, 0, 319, 239};

/* Snapshot counter */
static  volatile  unsigned int  lshotc = 1;

/* Snapshot file stream */
static  FILE   *lfiles = NULL;

/* Snapshot name */
static  unsigned char  lfname[SNAPSHOT_FILENAME_MAX+2] = "";
/* Snapshot path */
static  unsigned char  lfpath[SNAPSHOT_FILENAME_MAX+2] = "";




/*
 * Tmp var 
 */
/* Bitmap file header */
static  BITMAPFILEHEADER    lshead = { 0 };     

/* Bitmap information header */
static  BITMAPINFOHEADER    linfoh = { 0 };



int  in_snapshot_set_rect(GUI_RECT  *rect)
{
    if ( rect == NULL )
        return  -1;

    if ( rect->left > rect->right )
        return  -1;
    if ( rect->top  > rect->bottom )
        return  -1;

    lshotr = *rect;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  snapshot_set_rect(GUI_RECT  *rect)
{
    int  ret = 0;

    gui_lock( );
    ret = in_snapshot_set_rect(rect);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#ifdef  _LG_WINDOW_
int  in_win_snapshot_set_hwnd(/* HWND */ void *hwnd)
{
    HWND  p   = (HWND)hwnd;
    HDC   hdc = NULL;


    if ( p == NULL )
        return  -1;


     hdc = in_hdc_get_window(p);
     if ( hdc == NULL )
        return  -1;


    lshotr.left   = hdc->rect.left; 
    lshotr.top    = hdc->rect.top; 
    lshotr.right  = hdc->rect.right; 
    lshotr.bottom = hdc->rect.bottom; 

    in_hdc_release_win(p, hdc);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_snapshot_set_hwnd(/* HWND */ void *hwnd)
{
    int  ret = 0;

    gui_lock( );
    ret = in_win_snapshot_set_hwnd(hwnd);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */
#endif  /* _LG_WINDOW_ */

int  in_snapshot_get(void)
{
    GUI_COLOR     gui_color    = GUI_BLACK;
    SCREEN_COLOR  screen_color = 0;

    int        ret   = 0;
    int        i     = 0;
    int        j     = 0;
    int        tmp   = 0;



    if ( (lscrn.input_pixel) == NULL )
        return  -1;
    if ( lshotr.left > lshotr.right )
        return  -1;
    if ( lshotr.top  > lshotr.bottom )
        return  -1;


    /* Create file */
    memset(lfname, 0, sizeof(lfname));
    sprintf((char *)lfname, "%sLearningGUI-%03d.bmp", lfpath, lshotc);

    lfiles = stream_open_file((char *)lfname, "wb+");
    if ( lfiles == NULL )
        return  -1;


    /* 
     * Write file header 
     */
    memset(&lshead, 0, sizeof(lshead));
    lshead.bfType      = 'B' | (('M') << 8);

    /* 4 align for horizon */
    lshead.bfSize      = 14 + 40 + (GUI_RECTW(&lshotr))*(GUI_RECTH(&lshotr))*3;
    tmp = (GUI_RECTW(&lshotr)*3)%4;
    if ( tmp > 0 )
        lshead.bfSize += (4 - tmp)*(GUI_RECTH(&lshotr));

    lshead.bfReserved1 = 0;
    lshead.bfReserved2 = 0;
    lshead.bfOffBits   = 14 + 40;


    /* bfType: 2 bytes */ 
    ret = stream_write_file(lfiles, &(lshead.bfType), 2);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* bfSize: 4 bytes */ 
    ret = stream_write_file(lfiles, &(lshead.bfSize), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* bfReserved1 and bfReserved2: 4 bytes */ 
    ret = stream_write_file(lfiles, &(lshead.bfReserved1), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* bfOffBits: 4 bytes */ 
    ret = stream_write_file(lfiles, &(lshead.bfOffBits), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }


    /* 
     * Write file information header 
     */
    memset(&linfoh, 0, sizeof(linfoh));
    linfoh.biSize          = 40;
    linfoh.biWidth         = GUI_RECTW(&lshotr);
    linfoh.biHeight        = GUI_RECTH(&lshotr);
    linfoh.biPlanes        = 1;
    linfoh.biBitCount      = 24;
    linfoh.biCompression   = 0;
    linfoh.biSizeImage     = (GUI_RECTW(&lshotr))*(GUI_RECTH(&lshotr))*3;
    linfoh.biXPelsPerMeter = 0;
    linfoh.biYPelsPerMeter = 0;
    linfoh.biClrUsed       = 0;
    linfoh.biClrImportant  = 0;

    /* biSize: 4 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biSize), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biWidth: 4 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biWidth), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biHeight: 4 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biHeight), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biPlanes: 2 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biPlanes), 2);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biBitCount: 2 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biBitCount), 2);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biCompression: 4 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biCompression), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biSizeImage: 4 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biSizeImage), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biXPelsPerMeter: 4 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biXPelsPerMeter), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biYPelsPerMeter: 4 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biYPelsPerMeter), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biClrUsed: 4 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biClrUsed), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }

    /* biClrUsed: 4 bytes */ 
    ret = stream_write_file(lfiles, &(linfoh.biClrImportant), 4);
    if ( ret < 1 )
    {
        stream_close_file(lfiles);
        return  -1;
    }


    /* 
     * Write data 
    */
    (lscrn.input_sequence_start)();

    for ( i = GUI_RECTH(&lshotr) - 1; i >= 0; i-- )
    { 
        for ( j = 0; j < GUI_RECTW(&lshotr); j++ )
        {
            (lscrn.input_pixel)(lshotr.left + j , lshotr.top + i , &screen_color);
            gui_color = (lscrn.screen_to_gui_color)(screen_color);

            stream_write_file(lfiles, &gui_color, 3);
        }

        /* Padding */
        tmp = ((GUI_RECTW(&lshotr))*3)%4; 
        if ( tmp == 0 )
            continue;
   
        gui_color = GUI_BLACK; 
        stream_write_file(lfiles, &gui_color, 4 - tmp);
    }

    (lscrn.input_sequence_end)();

    /* Close file */
    stream_close_file(lfiles);

    /* Add counter */
    lshotc++;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  snapshot_get(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_snapshot_get();
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_snapshot_set_path(const char *path)
{
    unsigned int  counter = 0;


    if ( path == NULL )
        return  -1;

    memset(lfpath, 0, sizeof(lfpath));
    strncpy((char *)lfpath, path, SNAPSHOT_FILENAME_MAX);

    /* Deal with  adding "/" */
    counter = strlen((const char *)lfpath);
    if ( counter < 1 )
        return  1;

    if ( lfpath[counter-1] != '/' )
        strcat((char *)lfpath, "/");


    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  snapshot_set_path(const char *path)
{
    int  ret = 0;

    gui_lock( );
    ret = in_snapshot_set_path(path);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_FILE_SYSTEM_ */
#endif  /* _LG_SNAPSHOT_ */
#endif  /* _LG_SCREEN_ */
