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
#include  <cursor.h>

#include  <default.h>
#include  <screen.h>

#include  <dc.h>


#ifdef  _LG_DC_

/*
 * Basic DC array
 * the  0 index  is  SCREEN_DC_TYPE
 * the other indexes are MEMORY_DC_TYPE or BASIC_DC_TYPE
 */
volatile  GUI_DC   lbasdc[MAX_DC_NUM] = {{0}};

/* Every Basic DC default font */
volatile  GUI_FONT  *lbdcft = NULL;
 

HDC  in_hdc_get_basic(void)
{
    int  i = 0;

    for ( i = 1; i < MAX_DC_NUM; i++ )
    {
        if ( lbasdc[i].used == GUI_UNUSED )
        {
            lbasdc[i].used = GUI_USED;
            #ifdef  _LG_ALPHA_BLEND_
            lbasdc[i].is_alpha_blend = 0;
            #endif

            return  (HDC)(&lbasdc[i]); 
        }
    }
  
    #ifdef  _LG_ALPHA_BLEND_
    lbasdc[0].is_alpha_blend = 0;
    #endif

    return  (HDC)(&lbasdc[0]);
}

#ifndef  _LG_ALONE_VERSION_
HDC  hdc_get_basic(void)
{
    HDC  hdc = NULL;

    gui_lock( );
    hdc = in_hdc_get_basic( );
    gui_unlock( );

    return  hdc;
}
#endif

int   in_hdc_release_basic(HDC hdc)
{
    if ( hdc == HDC_DESKTOP_BASIC )
        return  1;

    in_hdc_basic_set_default(hdc);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int   hdc_release_basic(HDC hdc)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_release_basic(hdc);
    gui_unlock( );

    return  ret;
}
#endif

unsigned int  in_hdc_get_current_group(HDC hdc)
{
    unsigned int  group = INACTIVE_GROUP;


    if (hdc == NULL)
        return  DISABLED_GROUP;

    group = hdc->cur_group;
    if (group > MAX_COLOR_GROUP)
        group = MAX_COLOR_GROUP;

    return  group;
}

#ifndef  _LG_ALONE_VERSION_
unsigned int  hdc_get_current_group(HDC hdc)
{
    unsigned int  ret = INACTIVE_GROUP;

    gui_lock( );
    ret = in_hdc_get_current_group(hdc);
    gui_unlock( );

    return  ret;
}
#endif

int  in_hdc_set_current_group(HDC hdc, unsigned int group)
{
    if (hdc == NULL)
        return  -1;

    if (group > MAX_COLOR_GROUP)
        group = MAX_COLOR_GROUP;

    hdc->cur_group = group;
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_set_current_group(HDC hdc, unsigned int group)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_set_current_group(hdc,group);
    gui_unlock( );

    return  ret;
}
#endif


GUI_COLOR  in_hdc_get_group_color(HDC hdc, unsigned int group, unsigned int role)
{
    if (hdc == NULL)
        return  GUI_BLACK;

    if ( group > MAX_COLOR_GROUP )
        group = MAX_COLOR_GROUP;

    if ( role > MAX_COLOR_ROLE )
        role = MAX_COLOR_ROLE;

    return  ((HDC)hdc)->color[group][role];
}

#ifndef  _LG_ALONE_VERSION_
GUI_COLOR  hdc_get_group_color(HDC hdc, unsigned int group, unsigned int role)
{
    GUI_COLOR  ret = GUI_BLACK;

    gui_lock( );
    ret = in_hdc_get_group_color(hdc, group, role);
    gui_unlock( );

    return  ret;
}
#endif

int  in_hdc_set_group_color(HDC hdc, unsigned int group, unsigned int role, GUI_COLOR color)
{
    GUI_COLOR  tmp_color = GUI_BLACK;


    if ( group > MAX_COLOR_GROUP )
        group = MAX_COLOR_GROUP;

    if ( role > MAX_COLOR_ROLE )
        role = MAX_COLOR_ROLE;


    tmp_color  = ((HDC)hdc)->color[group][role];
    tmp_color &= 0xFF000000;
    tmp_color |= (color&0xFFFFFF);

    ((HDC)hdc)->color[group][role] = tmp_color;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_set_group_color(HDC hdc, unsigned int group, unsigned int role, GUI_COLOR color)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_set_group_color(hdc, group, role, color);
    gui_unlock( );

    return  ret;
}
#endif

int  in_hdc_set_back_mode(HDC hdc, int mode)
{
    if (hdc == NULL)
        return  -1;

    hdc->back_mode = mode;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_set_back_mode(HDC hdc, int mode)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_set_back_mode(hdc, mode);
    gui_unlock( );

    return  ret;
}
#endif

int  in_hdc_get_back_mode(HDC hdc)
{
    int  mode = 0;

    if (hdc == NULL)
        return  -1;

    mode = hdc->back_mode;

    return  mode;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_get_back_mode(HDC hdc)
{
    int  mode = 0;

    gui_lock( );
    mode = in_hdc_get_back_mode(hdc);
    gui_unlock( );

    return  mode;
}    
#endif


int  in_hdc_get_rect(HDC hdc, void *rect)
{
    if ( rect == NULL )
        return  -1;

    *((GUI_RECT *)rect) = hdc->rect;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_get_rect(HDC hdc, void *rect)
{
    int  ret;

    gui_lock( );
    ret = in_hdc_get_rect(hdc, rect);
    gui_unlock( );

    return  ret;
}    
#endif


int  in_hdc_set_rect(HDC hdc, void *rect)
{
    if ( rect == NULL )
        return  -1;

    hdc->rect = *((GUI_RECT *)rect);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_set_rect(HDC hdc, void *rect)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_set_rect(hdc, rect);
    gui_unlock( );

    return  ret;
}    
#endif


#ifdef  _LG_FONT_

int  in_hdc_set_font(HDC hdc, const void *font)
{
    if ( hdc == NULL )
        return  -1;

    hdc->font = (GUI_FONT *)font;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_set_font(HDC hdc, const void *font)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_set_font(hdc, font);
    gui_unlock( );

    return  ret;
}
#endif

int  in_hdc_get_font(HDC hdc, void *font)
{
    if ( font == NULL )
        return  -1;

    font = hdc->font;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_get_font(HDC hdc, void *font)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_get_font(hdc, font);
    gui_unlock( );

    return  ret;
}
#endif

int  in_hdc_get_font_width(HDC hdc)
{
    GUI_FONT   *font = NULL;


    font = hdc->font;
    if (font == NULL)
        return  -1;


    #ifdef  _LG_MONO_CHARSET_FONT_
    if ( ((font)->type) == MONO_CHARSET_FONT_TYPE )
        return  ((MONO_CHARSET_FONT *)(font->font))->width;
    #endif

    #ifdef  _LG_MONO_DISCRETE_FONT_
    if ( ((font)->type) == MONO_DISCRETE_FONT_TYPE )
        return  ((MONO_DISCRETE_FONT *)(font->font))->width;
    #endif

    #ifdef  _LG_MIXED_CHARSET_FONT_
    if ( ((font)->type) == MIXED_CHARSET_FONT_TYPE )
        return  (((MIXED_CHARSET_FONT *)(font->font))->mixed_char)->width;
    #endif

    #ifdef  _LG_MIXED_DISCRETE_FONT_
    if ( ((font)->type) == MIXED_DISCRETE_FONT_TYPE )
        return  ((MIXED_DISCRETE_UCHAR2 *)(((MIXED_DISCRETE_FONT *)(font->font))->list))->width;
    #endif

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_get_font_width(HDC hdc)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_get_font_width(hdc);
    gui_unlock( );

    return  ret;
}
#endif

int  in_hdc_get_font_height(HDC hdc)
{
    GUI_FONT   *font = NULL;


    if (hdc == NULL)
        return  -1;

    font = hdc->font;
    if (font == NULL)
        return  -1;

    #ifdef  _LG_MONO_CHARSET_FONT_
    if ( ((font)->type) == MONO_CHARSET_FONT_TYPE )
        return  ((MONO_CHARSET_FONT *)(font->font))->height;
    #endif

    #ifdef  _LG_MONO_DISCRETE_FONT_
    if ( ((font)->type) == MONO_DISCRETE_FONT_TYPE )
        return  ((MONO_DISCRETE_FONT *)(font->font))->height;
    #endif

    #ifdef  _LG_MIXED_CHARSET_FONT_
    if ( ((font)->type) == MIXED_CHARSET_FONT_TYPE )
        return  (((MIXED_CHARSET_FONT *)(font->font))->mixed_char)->height;
    #endif

    #ifdef  _LG_MIXED_DISCRETE_FONT_
    if ( ((font)->type) == MIXED_DISCRETE_FONT_TYPE )
        return  ((MIXED_DISCRETE_UCHAR2 *)(((MIXED_DISCRETE_FONT *)(font->font))->list))->height;
    #endif

    return  -1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_get_font_height(HDC hdc)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_get_font_height(hdc);
    gui_unlock( );

    return  ret;
}
#endif
#endif  /* _LG_FONT_ */


#ifdef  _LG_TEXT_METRIC_

int  in_hdc_get_text_metric(/* HDC hdc */ void *hdc, /* TEXT_METRIC *tm */ void *tm)
{
    if ( hdc == NULL )
        return  -1;

    *(GUI_TEXT_METRIC *)tm = ((HDC)hdc)->text_metric;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_get_text_metric(/* HDC hdc */ void *hdc, /* TEXT_METRIC *tm */void *tm)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_get_text_metric(hdc, tm);
    gui_unlock( );

    return  ret;
}
#endif

int  in_hdc_set_text_metric(/* HDC hdc */ void *hdc, /* TEXT_METRIC *tm */ void *tm)
{
    if (hdc == NULL)
        return  -1;

    ((HDC)hdc)->text_metric = *((GUI_TEXT_METRIC *)tm);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_set_text_metric(/* HDC hdc */ void *hdc, /* TEXT_METRIC *tm */ void *tm)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_set_text_metric(hdc, tm);
    gui_unlock( );

    return  ret;
}
#endif
    
#endif  /* _LG_TEXT_METRIC_ */

        
#ifndef  _LG_WINDOW_
int  in_hdc_clear(HDC hdc)
{
    GUI_COLOR     gui_color    = GUI_BLACK;
    SCREEN_COLOR  screen_color = GUI_BLACK;
    int  ret  = 0;


    if ( hdc == NULL )
        return  -1;

    if ( (lscrn.clear) == NULL )
        return  -1;
    if ( (lscrn.gui_to_screen_color) == NULL )
        return  -1;


    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
    #endif 
    #endif

    gui_color    = in_hdc_get_back_color(hdc);
    screen_color = lscrn.gui_to_screen_color(gui_color);
    ret = (lscrn.clear)(screen_color);

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_clear(HDC hdc)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_clear(hdc);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_WINDOW_ */


#ifdef  _LG_ALPHA_BLEND_

int  in_hdc_enable_disable_alpha(/* HDC hdc */ void *hdc, unsigned char enable)
{
    HDC  thdc = (HDC)hdc;


    if ( thdc == NULL )
        return  -1;
 

    if ( enable > 1 )
        enable = 1;

    thdc->is_alpha_blend = enable;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_enable_disable_alpha(/* HDC hdc */ void *hdc, unsigned char enable)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_enable_disable_alpha(hdc, enable);
    gui_unlock( );

    return  ret;
}
#endif
 
int  in_hdc_set_alpha_op_mode(/* HDC hdc */ void *hdc, unsigned char  mode)
{
    HDC  thdc = (HDC)hdc;


    if ( thdc == NULL )
        return  -1;
 

    if ( mode > MAX_ALPHA_BLEND_OP )
        mode = ALPHA_BLEND_OP_ADD;

    thdc->alpha_blend_op_mode = mode;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_set_alpha_op_mode(/* HDC hdc */ void *hdc, unsigned char  mode)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_set_alpha_op_mode(hdc, mode);
    gui_unlock( );

    return  ret;
}
#endif
     
int  in_hdc_get_alpha_op_mode(/* HDC hdc */ void *hdc, unsigned char *mode)
{
    HDC  thdc = (HDC)hdc;
    unsigned char tmode = 0;


    if ( thdc == NULL )
        return  -1; 
    if ( mode == NULL )
        return  -1;


    tmode = thdc->alpha_blend_op_mode;
    if ( tmode > MAX_ALPHA_BLEND_OP )
        tmode = ALPHA_BLEND_OP_ADD;

    *mode = tmode;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_get_alpha_op_mode(/* HDC hdc */ void *hdc, unsigned char *mode)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_get_alpha_op_mode(hdc, mode);
    gui_unlock( );

    return  ret;
}
#endif


int  in_hdc_set_alpha_value(/* HDC hdc */ void *hdc, unsigned char  alpha)
{
    HDC    thdc = (HDC)hdc;
    BUINT  i    = 0;
    BUINT  j    = 0;
    unsigned char tmp_alpha = alpha;


    if ( thdc == NULL )
        return  -1;


    if ( alpha == 0 )
    {
        (thdc->is_alpha_blend) = 0;
        return  1;
    }

    for ( i = 0; i < (MAX_COLOR_GROUP+1) ; i++ )
    {
        for ( j = 0; j < (MAX_COLOR_ROLE+1) ; j++ )
        {
            (thdc->color[i][j]) &= 0xFFFFFF;
            (thdc->color[i][j]) |= ((tmp_alpha&0xFF) << 24);
        }
    }    
    (thdc->is_alpha_blend) = 1;


    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_set_alpha_value(/* HDC hdc */ void *hdc, unsigned char  alpha)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_set_alpha_value(hdc, alpha);
    gui_unlock( );

    return  ret;
}
#endif
    
int  in_hdc_get_alpha_value(/* HDC hdc */ void *hdc, unsigned char *alpha)
{
    HDC    thdc     = (HDC)hdc;
    BUINT  group    = 0;
    GUI_COLOR color = GUI_BLACK;


    if ( thdc == NULL )
        return  -1;
    if ( alpha == NULL )
        return  -1;


    group = in_hdc_get_current_group(thdc);
    if ( group > MAX_COLOR_GROUP)
        group = 0;
            
    color = thdc->color[group][0];
    *alpha = ((color&0xFF000000) >> 24)&0xFF;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  hdc_get_alpha_value(/* HDC hdc */ void *hdc, unsigned char *alpha)
{
    int  ret = 0;

    gui_lock( );
    ret = in_hdc_get_alpha_value(hdc, alpha);
    gui_unlock( );

    return  ret;
}
#endif
 
#endif  /* _LG_ALPHA_BLEND_ */

#endif  /* _LG_DC_ */
