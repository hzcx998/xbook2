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

#include  <string.h>

#include  <lock.h>

#include  <screen.h>
#include  <mtjt.h>
#include  <image_icon.h>
#include  <dc.h>

#include  <cursor.h>


#ifdef  _LG_CURSOR_


#ifndef  MAX_CURSOR_WIDTH
#define  MAX_CURSOR_WIDTH                              16
#endif
#if (MAX_CURSOR_WIDTH < 8)
#undef   MAX_CURSOR_WIDTH
#define  MAX_CURSOR_WIDTH                              16
#endif
#if (MAX_CURSOR_WIDTH > 32)
#undef   MAX_CURSOR_WIDTH
#define  MAX_CURSOR_WIDTH                              16
#endif


#ifndef  MAX_CURSOR_HEIGHT
#define  MAX_CURSOR_HEIGHT                             16
#endif
#if (MAX_CURSOR_HEIGHT < 8)
#undef   MAX_CURSOR_HEIGHT
#define  MAX_CURSOR_HEIGHT                             16
#endif
#if (MAX_CURSOR_HEIGHT > 32)
#undef   MAX_CURSOR_HEIGHT
#define  MAX_CURSOR_HEIGHT                             16
#endif


#define  in_cursor_icon_fill(hdc, x, y, icon)    in_icon_fill_flag(hdc, x, y, icon, 1)    


#ifndef  _LG_COMMON_PALETTE_
/* Cursor shape data */
static  GUI_VAR_CONST  GUI_COLOR  mouse_normal_palette_color[] =
{
    0x00000000,0x00FFFFFF
};

static  GUI_VAR_CONST  GUI_PALETTE  mouse_normal_palette =
{
    /* .num      = */ sizeof(mouse_normal_palette_color)/sizeof(GUI_COLOR),
    /* .entries  = */ (GUI_COLOR *)mouse_normal_palette_color
};
#endif  /* _LG_COMMON_PALETTE_ */


static  GUI_VAR_CONST  unsigned char  mouse_normal_xor_index[] =
{
    0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x07,0x00,0x00,0x00,
    0x06,0x00,0x00,0x00,0x4E,0x00,0x00,0x00,0x6C,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,
    0x7E,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x70,0x00,0x00,0x00,
    0x60,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static  GUI_VAR_CONST  unsigned char  mouse_normal_and_data[] =
{
    0xFC,0xFF,0x00,0x00,0xF8,0x7F,0x00,0x00,0xF8,0x7F,0x00,0x00,0x70,0x7F,0x00,0x00,
    0x30,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x3F,0x00,0x00,
    0x00,0x7F,0x00,0x00,0x00,0xFF,0x00,0x00,0x01,0xFF,0x00,0x00,0x03,0xFF,0x00,0x00,
    0x07,0xFF,0x00,0x00,0x0F,0xFF,0x00,0x00,0x1F,0xFF,0x00,0x00,0x3F,0xFF,0x00,0x00
};

static  GUI_VAR_CONST  GUI_ICON  mouse_normal_icon =
{
    /* .type              = */ CURSOR_TYPE,
    /* .width             = */ 16,
    /* .height            = */ 16,
    /* .bits_per_pixel    = */ 1,
    /* .bytes_per_line    = */ 4,
    /* .left              = */ 0,
    /* .top               = */ 0,
    /* .bit16_format      = */ 0,
    /* .is_rle8_format    = */ 0,
    /* .panes_left        = */ 0,
    /* .bpp_top           = */ 0,
    #ifndef  _LG_COMMON_PALETTE_
    /* .palette           = */ (GUI_PALETTE *)(&mouse_normal_palette),
    #else
    /* .palette           = */ (GUI_PALETTE *)(&l1pal),
    #endif  /* _LG_COMMON_PALETTE_ */
    /* .xor_data          = */ (unsigned char *)mouse_normal_xor_index,
    /* .and_data          = */ (unsigned char *)mouse_normal_and_data
};


/* Raw screen data block covered by cursor */
static  SCREEN_COLOR  lcurda[MAX_CURSOR_HEIGHT][MAX_CURSOR_WIDTH] = { {0, 0} };

/* Cursor position information */
/* Cursor x position */
static  volatile          int   lcuxva = 0;
/* Cursor y position */
static  volatile          int   lcuyva = 0;

/* Cursor width */
static  volatile unsigned int   lcuwid = MAX_CURSOR_WIDTH;
/* Cursor height */
static  volatile unsigned int   lcuhei = MAX_CURSOR_HEIGHT;


/* Cursor init flag */
static            unsigned int  lcuini = 0;

/* Show cursor flag */
static  volatile  unsigned int  lcufla = 1;


/* Refresh cursor flag */
static  volatile  unsigned int  lcuref = 0;


/* Mouse shape */
static  GUI_ICON  *lmoush[MAX_CURSORS] = { NULL }; 

/* Cursor resouce id */
static  volatile  BUINT lcurid = 0;



/* Cursor data function */
static  int  in_cursor_restore_back(void)
{
    unsigned int  i = 0;
    unsigned int  j = 0;


    for ( i = 0; i < lcuhei; i++ )
    {
        for ( j = 0; j < lcuwid; j++ )
        {
            (lscrn.output_pixel)(lcuxva+j, lcuyva+i, lcurda[i][j]);
        }
    }

    return  1;
}

static  int  in_cursor_save_back(void)
{ 
    unsigned int  i = 0;
    unsigned int  j = 0;

            
    if ( (lscrn.input_pixel) == NULL )
        return  -1;

    for ( i = 0; i < lcuhei; i++ )
    {
        for ( j = 0; j < lcuwid; j++ )
        {
            (lscrn.input_pixel)(lcuxva+j, lcuyva+i, &(lcurda[i][j]));
        }
    }

    return  1;
}


/* Cursor init */
int  in_cursor_init(void)
{
    unsigned int  i = 0;


    for ( i = 0; i < MAX_CURSORS; i++ )
        lmoush[i] = (GUI_ICON *)&mouse_normal_icon;

    memset(lcurda, 0, sizeof(lcurda));
    lcuxva = 0;
    lcuyva = 0;
    lcuwid = MAX_CURSOR_WIDTH;
    lcuhei = MAX_CURSOR_HEIGHT;

    lcuini = 0;

    lcurid = 0;
    lcufla = 1;
    lcuref = 0;
 
    return  1;
}
    

/** 
 ** Cursor common APIs 
 **/

/* Enable cursor */
int  in_cursor_enable(void)
{
    int  ret = 0;

    lcufla = 1;

    ret = in_cursor_show();

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  cursor_enable(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_enable();
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


/* Disable cursor */
int  in_cursor_disable(void)
{
    int  ret = 0;

    lcufla = 0;

    ret = in_cursor_hide();

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  cursor_disable(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_disable();
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

/* Show cursor */
int  in_cursor_show(void)
{
    int  ret = 0;


    if ( lcufla < 1 )
        return  0;

    if ( lcuini > 0 )
        in_cursor_restore_back( );

    ret = in_cursor_save_back();
    #ifdef  _LG_ICON_
    if  ( lcurid > (MAX_CURSORS-1) )
        lcurid = 0;

    ret = in_cursor_icon_fill(HDC_DESKTOP_BASIC, lcuxva, lcuyva, (void *)lmoush[lcurid]);
    #endif
    lcuini = 1;

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  cursor_show(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_show();
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


/* Hide cursor */
int  in_cursor_hide(void)
{
    int  ret = 0;


    if ( lcuini > 0 )
        in_cursor_restore_back( );

    lcuxva = (lmtjt.cur_x);
    lcuyva = (lmtjt.cur_y);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  cursor_hide(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_hide();
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


/* Cursor position */
int  in_cursor_get_position(void *point)
{
    GUI_POINT  *p = (GUI_POINT *)point;


    if ( p == NULL )
        return  -1;

    p->x = (lmtjt.cur_x);
    p->y = (lmtjt.cur_y);

    if ( p->x < 0 )
        p->x = 0;
    if ( p->x > ((lscrn.width) - 1) )
        p->x = ((lscrn.width) - 1);

    if ( p->y < 0 )
        p->y = 0;
    if ( p->y > ((lscrn.height) - 1) )
        p->y = ((lscrn.height) - 1);


    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  cursor_get_position(void *point)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_get_position(point);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_cursor_set_position(int x, int y)
{
    int  ret = 0;


    if ( x < 0 )
        x = 0;
    if ( x > ((lscrn.width) - 1) )
        x = ((lscrn.width) - 1);

    if ( y < 0 )
        y = 0;
    if ( y > ((lscrn.height) - 1) )
        y = ((lscrn.height) - 1);


    if ( lcufla < 1 )
        return  0;

    if ( lcuini > 0 )
        in_cursor_restore_back( );


    (lmtjt.cur_x) = x;
    (lmtjt.cur_y) = y;

    lcuxva = x;
    lcuyva = y;


    ret = in_cursor_save_back();
    #ifdef  _LG_ICON_
    if  ( lcurid > (MAX_CURSORS-1) )
        lcurid = 0;

    ret = in_cursor_icon_fill(HDC_DESKTOP_BASIC, lcuxva, lcuyva, (void *)lmoush[lcurid]);
    #endif
    lcuini = 1;

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  cursor_set_position(int x, int y)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_set_position(x, y);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


/* Set cursor shape */
int  in_cursor_set_shape(unsigned int cursor_id, const void *shape)
{
    GUI_ICON  *p = (GUI_ICON *)shape;
    int    delta = 0;
    int      ret = 0;


    if ( cursor_id > (MAX_CURSORS-1) )
        return  -1;
    if ( p == 0 )
        return  -1;


    if ( lcufla > 0 )
    {
        if ( lcuini > 0 )
            in_cursor_restore_back( );
    }

    lmoush[cursor_id] = p;

    delta = (lscrn.width) - lcuxva;
    if ( delta > lmoush[cursor_id]->width )
        lcuwid = lmoush[cursor_id]->width;
    else
        lcuwid = delta;

    if ( lcuwid > MAX_CURSOR_WIDTH )
        lcuwid = MAX_CURSOR_WIDTH;

    delta = (lscrn.height) - lcuyva;
    if ( delta > lmoush[cursor_id]->height )
        lcuhei = lmoush[cursor_id]->height;
    else
        lcuhei = delta;

    if ( lcuhei > MAX_CURSOR_HEIGHT )
        lcuhei = MAX_CURSOR_HEIGHT;


    in_cursor_show();

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  cursor_set_shape(unsigned int cursor_id, const void *shape)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_set_shape(cursor_id, shape);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

    
/* Get cursor id */    
int  in_cursor_get_id(void)
{
    return  lcurid;
}

#ifndef  _LG_ALONE_VERSION_
int  cursor_get_id(void)
{
    int  ret = 0;

    gui_lock( );
    ret = cursor_get_id();
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

/* Set cursor id */    
int  in_cursor_set_id(unsigned int cursor_id)
{
    if ( cursor_id > (MAX_CURSORS-1) )
        return  -1;

    lcurid = cursor_id;

    return  lcurid;
}

#ifndef  _LG_ALONE_VERSION_
int  cursor_set_id(unsigned int cursor_id)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_set_id(cursor_id);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

   

/* Absolute coordinate */
int  in_cursor_maybe_restore_back_abs(int left, int top, int right, int bottom)
{
    if (lcufla < 1)
        return  0;


    lcuref = 0;

    /* In cursor rectangle ? */
    if ( left > right )
        return  0;
    if ( top > bottom )
        return  0;


    if (left > (lcuxva + lcuwid - 1))
        return  0;
    if (right < lcuxva)
        return  0;

    if (top > (lcuyva + lcuhei - 1))
        return  0;
    if (bottom < lcuyva)
        return  0;


    /* Restore back */
    if ( lcuini > 0 )
        in_cursor_restore_back( );

    lcuini = 1;

    lcuref = 1;

    return  1;
}

#ifdef  _LG_WINDOW_
#ifndef  _LG_ALONE_VERSION_
int  cursor_maybe_restore_back_abs(int left, int top, int right, int bottom)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_maybe_restore_back_abs(left, top, right, bottom);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */
#endif  /*  _LG_WINDOW_ */

int  in_cursor_maybe_refresh(void)
{
    if (lcufla < 1)
        return  0;

    if (lcuref < 1)
        return  0;

    in_cursor_save_back();
    #ifdef  _LG_ICON_
    if  ( lcurid > (MAX_CURSORS-1) )
        lcurid = 0;

    in_cursor_icon_fill(HDC_DESKTOP_BASIC, lcuxva, lcuyva, (void *)lmoush[lcurid]);
    #endif
    lcuini = 1;

    lcuref = 0;

    return  1;
}

#ifdef  _LG_WINDOW_
#ifndef  _LG_ALONE_VERSION_
int  cursor_maybe_refresh(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_cursor_maybe_refresh();
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */
#endif  /*  _LG_WINDOW_ */

#endif  /* _LG_CURSOR_ */
