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

#ifndef  __LGUI_DC_HEADER__
#define  __LGUI_DC_HEADER__        1

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


/* DC type */
#define  DESKTOP_DC_TYPE                0x00
#define  SYSTEM_DC_TYPE                 0x01
#define  MEMORY_DC_TYPE                 0x02
#define  PRINTER_DC_TYPE                0x03
#define  OTHER_DC_TYPE                  0x04


/* Painting modes */
#define  MODE_COPY                      0x00       /* src */
#define  MODE_TRANSPARENCY              0x01       /* dst */
#define  MODE_XOR                       0x02       /* src ^ dst */
#define  MODE_OR                        0x03       /* src | dst */
#define  MODE_AND                       0x04       /* src & dst */
#define  MODE_OR_REVERSE                0x05       /* src | ~dst */
#define  MODE_AND_REVERSE               0x06       /* src & ~dst */
#define  MODE_XOR_FGBG                  0x07       /* src ^ background ^ dst (This is the Java XOR mode) */
#define  MODE_EQUIV                     0x08       /* ~(src ^ dst) */
#define  MODE_NOR                       0x09       /* ~(src | dst) */
#define  MODE_NAND                      0x0A       /* ~(src & dst) */
#define  MODE_INVERT	 	            0x0B       /* ~dst */
#define  MODE_COPY_INVERTED             0x0C       /* ~src */
#define  MODE_OR_INVERTED               0x0D       /* ~src | dst */
#define  MODE_AND_INVERTED              0x0E       /* ~src & dst */
#define  MODE_CLEAR                     0x0F       /* 0 */


/* Alpha blend operation modes */
#define  ALPHA_BLEND_OP_ADD             0x00
#define  ALPHA_BLEND_OP_SUBTRACT        0x01
#define  ALPHA_BLEND_OP_REVSUBTRACT     0x02
#define  ALPHA_BLEND_OP_MIN             0x03
#define  ALPHA_BLEND_OP_MAX             0x04
#define  MAX_ALPHA_BLEND_OP             ALPHA_BLEND_OP_MAX


/* Desktop Basic DC */
#define  HDC_DESKTOP_BASIC              ((HDC)(&lbasdc[0]))



#ifdef  _LG_DC_

#ifdef  __cplusplus
extern  "C"
{
#endif


    extern  volatile  GUI_DC  lbasdc[MAX_DC_NUM];

    /* Every Basic DC default font */
    extern  volatile  GUI_FONT  *lbdcft;
 


    /* HDC function */
    HDC  in_hdc_get_basic(void);
    int  in_hdc_release_basic(HDC hdc);


    unsigned int in_hdc_get_current_group(HDC hdc);
    int          in_hdc_set_current_group(HDC hdc, unsigned int group);

    GUI_COLOR    in_hdc_get_group_color(HDC hdc, unsigned int group, unsigned int role);
    int          in_hdc_set_group_color(HDC hdc, unsigned int group, unsigned int role, GUI_COLOR color);

    int  in_hdc_get_back_mode(HDC hdc);
    int  in_hdc_set_back_mode(HDC hdc, int mode);

    int  in_hdc_get_rect(HDC hdc, void *rect);
    int  in_hdc_set_rect(HDC hdc, void *rect);

    #ifdef  _LG_FONT_
    int  in_hdc_set_font(HDC hdc, const void *font);
    int  in_hdc_get_font(HDC hdc, void *font);

    int  in_hdc_get_font_width(HDC hdc);
    int  in_hdc_get_font_height(HDC hdc);
    #endif

    #ifdef  _LG_TEXT_METRIC_
    int  in_hdc_get_text_metric(/* HDC hdc */ void *hdc, /* TEXT_METRIC *tm */ void *tm);
    int  in_hdc_set_text_metric(/* HDC hdc */ void *hdc, /* TEXT_METRIC *tm */ void *tm);
    #endif

    #ifndef  _LG_WINDOW_
    int  in_hdc_clear(HDC hdc);
    #endif

    #ifdef  _LG_ALPHA_BLEND_
    int  in_hdc_enable_disable_alpha(/* HDC hdc */ void *hdc, unsigned char enable);

    int  in_hdc_set_alpha_op_mode(/* HDC hdc */ void *hdc, unsigned char  mode);
    int  in_hdc_get_alpha_op_mode(/* HDC hdc */ void *hdc, unsigned char *mode);

    int  in_hdc_set_alpha_value(/* HDC hdc */ void *hdc, unsigned char  alpha);
    int  in_hdc_get_alpha_value(/* HDC hdc */ void *hdc, unsigned char *alpha);
    #endif


    #define  in_hdc_get_back_color(hdc)                        in_hdc_get_group_color(hdc, in_hdc_get_current_group(hdc), BACK_ROLE)
    #define  in_hdc_set_back_color(hdc,color)                  in_hdc_set_group_color(hdc, in_hdc_get_current_group(hdc), BACK_ROLE, color)

    #define  in_hdc_get_fore_color(hdc)                        in_hdc_get_group_color(hdc, in_hdc_get_current_group(hdc), FORE_ROLE)
    #define  in_hdc_set_fore_color(hdc,color)                  in_hdc_set_group_color(hdc, in_hdc_get_current_group(hdc), FORE_ROLE, color)


    #define  in_hdc_get_text_back_color(hdc)                   in_hdc_get_group_color(hdc, in_hdc_get_current_group(hdc), TEXT_BACK_ROLE)
    #define  in_hdc_set_text_back_color(hdc,color)             in_hdc_set_group_color(hdc, in_hdc_get_current_group(hdc), TEXT_BACK_ROLE, color)

    #define  in_hdc_get_text_fore_color(hdc)                   in_hdc_get_group_color(hdc, in_hdc_get_current_group(hdc), TEXT_FORE_ROLE)
    #define  in_hdc_set_text_fore_color(hdc,color)             in_hdc_set_group_color(hdc, in_hdc_get_current_group(hdc), TEXT_FORE_ROLE, color)


    #define  in_hdc_get_current_group_color(hdc,role)          in_hdc_get_group_color(hdc, in_hdc_get_current_group(hdc), role)
    #define  in_hdc_set_current_group_color(hdc,role,color)    in_hdc_set_group_color(hdc, in_hdc_get_current_group(hdc), role, color)


    #ifndef  _LG_ALONE_VERSION_

    /* HDC function */
    HDC  hdc_get_basic(void);
    int  hdc_release_basic(HDC hdc);


    unsigned int hdc_get_current_group(HDC hdc);
    int          hdc_set_current_group(HDC hdc, unsigned int group);

    GUI_COLOR    hdc_get_group_color(HDC hdc, unsigned int group, unsigned int role);
    int          hdc_set_group_color(HDC hdc, unsigned int group, unsigned int role, GUI_COLOR color);

    int  hdc_get_back_mode(HDC hdc);
    int  hdc_set_back_mode(HDC hdc, int mode);

    int  hdc_get_rect(HDC hdc, void *rect);
    int  hdc_set_rect(HDC hdc, void *rect);

    #ifdef  _LG_FONT_
    int  hdc_set_font(HDC hdc, const void *font);
    int  hdc_get_font(HDC hdc, void *font);

    int  hdc_get_font_width(HDC hdc);
    int  hdc_get_font_height(HDC hdc);
    #endif

    #ifdef  _LG_TEXT_METRIC_
    int  hdc_get_text_metric(/* HDC hdc */ void *hdc, /* TEXT_METRIC *tm */ void *tm);
    int  hdc_set_text_metric(/* HDC hdc */ void *hdc, /* TEXT_METRIC *tm */ void *tm);
    #endif

    #ifndef  _LG_WINDOW_
    int  hdc_clear(HDC hdc);
    #endif

    #ifdef  _LG_ALPHA_BLEND_
    int  hdc_enable_disable_alpha(/* HDC hdc */ void *hdc, unsigned char enable);

    int  hdc_set_alpha_op_mode(/* HDC hdc */ void *hdc, unsigned char  mode);
    int  hdc_get_alpha_op_mode(/* HDC hdc */ void *hdc, unsigned char *mode);

    int  hdc_set_alpha_value(/* HDC hdc */ void *hdc, unsigned char  alpha);
    int  hdc_get_alpha_value(/* HDC hdc */ void *hdc, unsigned char *alpha);

    #endif

    #else  /* _LG_ALONE_VERSION_ */

    #define  hdc_get_basic()                                  in_hdc_get_basic()
    #define  hdc_release_basic(hdc)                           in_hdc_release_basic(hdc)

    #define  hdc_get_current_group(hdc)                       in_hdc_get_current_group(hdc)
    #define  hdc_set_current_group(hdc,group)                 in_hdc_set_current_group(hdc,group)

    #define  hdc_get_group_color(hdc,group,role)              in_hdc_get_group_color(hdc,group,role)
    #define  hdc_set_group_color(hdc,group,role,color)        in_hdc_set_group_color(hdc,group,role,color)

    #define  hdc_get_back_mode(hdc)                           in_hdc_get_back_mode(hdc)
	#define  hdc_set_back_mode(hdc,mode)                      in_hdc_set_back_mode(hdc,mode) 

    #define  hdc_get_rect(hdc, rect)                          in_hdc_get_rect(hdc, rect)
    #define  hdc_set_rect(hdc, rect)                          in_hdc_set_rect(hdc, rect)

    #ifdef  _LG_FONT_
    #define  hdc_set_font(hdc, font)                          in_hdc_set_font(hdc, font)
    #define  hdc_get_font(hdc, font)                          in_hdc_get_font(hdc, font)

    #define  hdc_get_font_width(hdc)                          in_get_font_widht(hdc)
    #define  hdc_get_font_height(hdc)                         in_get_font_height(hdc)
    #endif

    #ifdef  _LG_TEXT_METRIC_
    #define  hdc_get_text_metric(hdc, tm)                     in_hdc_get_text_metric(hdc, tm)
    #define  hdc_set_text_metric(hdc, tm)                     in_hdc_set_text_metric(hdc, tm)
    #endif

    #ifndef  _LG_WINDOW_
    #define  hdc_clear(hdc)                                   in_hdc_clear(hdc)
    #endif

    #ifdef  _LG_ALPHA_BLEND_
    #define  hdc_enable_disable_alpha(hdc, enable)            in_hdc_enable_disable_alpha(hdc, enable)

    #define  hdc_set_alpha_mode(hdc, mode)                    in_hdc_set_alpha_mode(hdc,mode)
    #define  hdc_get_alpha_mode(hdc, mode)                    in_hdc_get_alpha_mode(hdc,mode)

    #define  hdc_set_alpha_value(hdc, alpha)                  in_hdc_set_alpha_value(hdc, alpha)
    #define  hdc_get_alpha_value(hdc, alpha)                  in_hdc_get_alpha_value(hdc, alpha)
    #endif


    #endif  /* _LG_ALONE_VERSION_ */


    #define  hdc_get_back_color(hdc)                          hdc_get_group_color(hdc, hdc_get_current_group(hdc), BACK_ROLE)
    #define  hdc_set_back_color(hdc,color)                    hdc_set_group_color(hdc, hdc_get_current_group(hdc), BACK_ROLE, color)

    #define  hdc_get_fore_color(hdc)                          hdc_get_group_color(hdc, hdc_get_current_group(hdc), FORE_ROLE)
    #define  hdc_set_fore_color(hdc,color)                    hdc_set_group_color(hdc, hdc_get_current_group(hdc), FORE_ROLE, color)

    #define  hdc_get_text_back_color(hdc)                     hdc_get_group_color(hdc, hdc_get_current_group(hdc), TEXT_BACK_ROLE)
    #define  hdc_set_text_back_color(hdc,color)               hdc_set_group_color(hdc, hdc_get_current_group(hdc), TEXT_BACK_ROLE, color)

    #define  hdc_get_text_fore_color(hdc)                     hdc_get_group_color(hdc, hdc_get_current_group(hdc), TEXT_FORE_ROLE)
    #define  hdc_set_text_fore_color(hdc,color)               hdc_set_group_color(hdc, hdc_get_current_group(hdc), TEXT_FORE_ROLE, color)

    #define  hdc_get_current_group_color(hdc,role)            hdc_get_group_color(hdc, hdc_get_current_group(hdc), role)
    #define  hdc_set_current_group_color(hdc,role,color)      hdc_set_group_color(hdc, hdc_get_current_group(hdc), role, color)

    #ifdef  _LG_ALPHA_BLEND_
    #define  hdc_enable_alpha(hdc)                            hdc_enable_disable_alpha(hdc, 1)
    #define  hdc_disable_alpha(hdc)                           hdc_enable_disable_alpha(hdc, 0)
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_DC_ */

    
#ifdef  _LG_WINDOW_
#define  hdc_clear(hdc)
#endif


#endif  /* __LGUI_DC_HEADER__ */
