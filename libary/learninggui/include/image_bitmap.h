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

#ifndef  __LGUI_IMAGE_BITMAP_HEADER__
#define  __LGUI_IMAGE_BITMAP_HEADER__

#include  <type_color.h>

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif

#include  <type_gui.h>


#include  <palette.h>



/* windows bitmap file structure */
/* 14 bytes */
struct tagBITMAPFILEHEADER 
{
    UINT16  bfType;
    UINT32  bfSize;
    UINT16  bfReserved1;
    UINT16  bfReserved2;
    UINT32  bfOffBits;
};
typedef  struct  tagBITMAPFILEHEADER  BITMAPFILEHEADER; 

/* 40 bytes */
struct tagBITMAPINFOHEADER
{
    UINT32  biSize;
     INT32  biWidth;
     INT32  biHeight;
    UINT16  biPlanes;
    UINT16  biBitCount;
    UINT32  biCompression;
    UINT32  biSizeImage;
     INT32  biXPelsPerMeter;
     INT32  biYPelsPerMeter;
    UINT32  biClrUsed;
    UINT32  biClrImportant;
};
typedef  struct  tagBITMAPINFOHEADER  BITMAPINFOHEADER; 


struct tagRGBQUAD 
{
    UCHAR   rgbBlue;
    UCHAR   rgbGreen;
    UCHAR   rgbRed;
    UCHAR   rgbReserved;
};
typedef  struct  tagRGBQUAD  RGBQUAD; 


struct  _GUI_BITMAP
{
          unsigned int    width;
          unsigned int    height;
          unsigned int    bits_per_pixel;
          unsigned int    bytes_per_line;
          GUI_COLOR       transparent_color;
          unsigned int    is_transparent;
          unsigned int    bit16_format;
          unsigned int    is_rle8_format;
    const GUI_PALETTE    *palette;
    const unsigned char  *data;
};
typedef  struct  _GUI_BITMAP   GUI_BITMAP;





#ifdef  _LG_BITMAP_

#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_bitmap_fill(HDC hdc, int x, int y, const void *bitmap);
    int  in_bitmap_rotate(HDC hdc, int x, int y, const void *bitmap, void *rotate);
    int  in_bitmap_rotate_special(HDC hdc, int x, int y, const void *bitmap, void *rotate);

    int  in_bitmap_symmetry_special(HDC hdc, int x, int y, const void *bitmap, void *symmetry);

    #ifdef   _LG_BITMAP_FILE_
    int  in_bitmap_file_fill(HDC hdc, int x, int y, const char *file);
    #endif

    #ifdef   _LG_FILL_BITMAP_EXTENSION_
    int  in_bitmap_scale(HDC hdc, const void *rect,  const void *bitmap); 
    #ifdef   _LG_BITMAP_FILE_
    int  in_bitmap_file_fill_rect(HDC hdc, const void *rect,  const char *file); 
    #endif
    #endif
        

    #ifndef  _LG_ALONE_VERSION_
    int  bitmap_fill(HDC hdc, int x, int y, const void *bitmap);
    int  bitmap_rotate(HDC hdc, int x, int y, const void *bitmap, void *rotate);
    int  bitmap_rotate_special(HDC hdc, int x, int y, const void *bitmap, void *rotate);

    int  bitmap_symmetry_special(HDC hdc, int x, int y, const void *bitmap, void *symmetry);

    #ifdef   _LG_BITMAP_FILE_
    int  bitmap_file_fill(HDC hdc, int x, int y, const char *file);
    #endif
    #else
    #define  bitmap_fill(hdc, x, y, bitmap)    in_bitmap_fill(hdc, x, y, bitmap)
    #define  bitmap_rotate(hdc,x,y,bitmap,rotate)  in_bitmap_rotate(hdc,x,y,bitmap,rotate)
    #define  bitmap_rotate_special(hdc,x,y,bitmap,rotate)  in_bitmap_rotate_simple(hdc,x,y,bitmap,rotate)

    #define  bitmap_symmetry_special(hdc,x,y,bitmap,symmetry)   in_bitmap_symmetry_special(hdc,x,y,bitmap,symmetry) 

    #ifdef   _LG_BITMAP_FILE_
    #define  bitmap_file_fill(hdc, x, y, file) in_bitmap_file_fill(hdc, x, y, file)
    #endif

    #endif

    #ifdef   _LG_FILL_BITMAP_EXTENSION_
    #ifndef  _LG_ALONE_VERSION_
    int  bitmap_scale(HDC hdc, const void *rect, const void *bitmap);
    #ifdef   _LG_BITMAP_FILE_
    int  bitmap_file_fill_rect(HDC hdc, const void *rect,  const char *file); 
    #endif
    #else
    #define  bitmap_scale(hdc, rect, bitmap)    in_bitmap_scale(hdc, rect, bitmap)
    #ifdef   _LG_BITMAP_FILE_
    #define  bitmap_file_fill_rect(hdc, rect, file) in_bitmap_file_fill_rect(hdc, rect, file)
    #endif
    #endif
    #endif


#ifdef  __cplusplus
}
#endif

#endif  /* _LG_BITMAP_ */

#endif  /* __LGUI_IMAGE_BITMAP_HEADER__ */
