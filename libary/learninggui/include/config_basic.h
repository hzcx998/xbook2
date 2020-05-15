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

/*
 * Created by Rao Youkun(960747373@qq.com) at 2011-12-28
 */
#ifndef  __LGUI_CONFIG_BASIC_HEADER__
#define  __LGUI_CONFIG_BASIC_HEADER__

/* Window support macro */
//#define  _LG_WINDOW_

/* Multi-thread support macro */
/* #define  _LG_MULTI_THREAD_ */

/* Color support macro */
#define  _LG_COLOR_
    /* For screen color bit deepth */
    #define  _LG_1_BIT_COLOR_
    #define  _LG_2_BIT_COLOR_
    #define  _LG_4_BIT_COLOR_
    #define  _LG_8_BIT_COLOR_
    #define  _LG_15_BIT_COLOR_
    #define  _LG_16_BIT_COLOR_
    #define  _LG_24_BIT_COLOR_
    #define  _LG_32_BIT_COLOR_

    /* Common palette data set */
    #define   _LG_COMMON_PALETTE_

    /* Palette routine */
    #define  _LG_PALETTE_ROUTINE_
        #define  _LG_8_BIT_SYSTEM_PALETTE_
        /* #define  _LG_RESTORE_SYSTEM_PALETTE_ */

    /* Screen color conversion routine */
    #define   _LG_COLOR_CONVERSION_
        /* Palette conversion */
        #define  _LG_PALETTE_CONVERSION_


/* DC support macro */
#define  _LG_DC_
    #define  MAX_DC_NUM                                    32

    #define   _LG_PEN_
    #define   _LG_BRUSH_

    #define  _LG_2D_
        #define   _LG_LINE_
        #define   _LG_RECTANGLE_
        #define   _LG_TRIANGLE_
        #define   _LG_CIRCLE_
        #define   _LG_ELLIPSE_

    #define  _LG_FONT_
        #define  FONT_NAME_LEN                             8
        #define  _LG_MONO_CHARSET_FONT_
        #define  _LG_MONO_DISCRETE_FONT_
        #define  _LG_MIXED_CHARSET_FONT_
        #define  _LG_MIXED_DISCRETE_FONT_

        #define  _LG_FONT_ID_

        #define  MISSING_CHAR_BLANK_WIDTH                  8

        #define _LG_ASCII_LATIN_D0816C_FONT_
        #define  _LG_ASCII_LATIN_D0612C_FONT_

        #define  _LG_MULTI_BYTE_CODE_VERSION_
            #define MULTI_BYTE_START_CODE                  0xA1

            /* #define  _LG_GB2312_D1616CS_C1_FONT_ */
                /*
                #define  INCLUDE_GB2312_D1616CS_SYSBOL
                #define  INCLUDE_GB2312_D1616CS_C2
                */

            /*
            #define  _LG_GB2312_D1212CS_C1_FONT_
                #define  INCLUDE_GB2312_D1212CS_SYSBOL
                #define  INCLUDE_GB2312_D1212CS_C2
            */

        /*
        #define  _LG_UNICODE_VERSION_
            #define   _LG_UNICODE_BIG_ENDIAN_

            #define   _LG_CJK_UNIFIED_D1616CS_FONT_
            #define   _LG_HZ1616_PUNCTUATION_

            #define   _LG_CJK_UNIFIED_D1212CS_FONT_
            #define   _LG_HZ1212_PUNCTUATION_
       */

    #define  _LG_TEXT_METRIC_
    #define  _LG_TEXT_OUT_EXTENSION_
    /* #define   _LG_TEXT_GLYPH_ */


    #define   _LG_BITMAP_
        #define  _LG_1_BIT_BITMAP_
        #define  _LG_2_BIT_BITMAP_
        #define  _LG_4_BIT_BITMAP_
        #define  _LG_8_BIT_BITMAP_
        #define  _LG_16_BIT_BITMAP_
        #define  _LG_24_BIT_BITMAP_
        #define  _LG_32_BIT_BITMAP_

        #define   _LG_BITMAP_FILE_
        #define   _LG_FILL_BITMAP_EXTENSION_


    #define   _LG_ICON_
        #define  _LG_1_BIT_ICON_
        #define  _LG_2_BIT_ICON_
        #define  _LG_4_BIT_ICON_
        #define  _LG_8_BIT_ICON_
        #define  _LG_16_BIT_ICON_
        #define  _LG_24_BIT_ICON_
        #define  _LG_32_BIT_ICON_

        #define   _LG_FILL_ICON_EXTENSION_

    #define   _LG_GIF_
        #define  _LG_1_BIT_GIF_
        #define  _LG_2_BIT_GIF_
        #define  _LG_4_BIT_GIF_
        #define  _LG_8_BIT_GIF_

/* Alpha support macro */
#define  _LG_ALPHA_BLEND_
 

/* Screen support macro */
#define  _LG_SCREEN_
    /* #define  _LG_SNAPSHOT_ */
     #define  _LG_NEED_REFRESH_SCREEN_


/* Keyboard support macro */
#define   _LG_KEYBOARD_ 
   
/* MTJT: Mouse-Touchscreen-Joystick-Tablet */
/* MTJT support macro */
#define   _LG_MTJT_ 

/* Cursor support macro */    

#define  _LG_CURSOR_
    #define  MAX_CURSOR_WIDTH                              16
    #define  MAX_CURSOR_HEIGHT                             16

/* Message support macro */
#define  _LG_MESSAGE_
    #define  MESSAGE_QUEUE_LEN                             128
    #define  _LG_LONG_MESSAGE_

    #define   _LG_COUNTER_
        #define   MAX_COUNTER                              8

    /* microseconds */
    /*
    #define  _LG_TIMER_
        #define   MAX_TIMER                                8
        #define   TIMER_OFFSET                             75
    */

/* Filesystem support macro */
/* #define  _LG_FILE_SYSTEM_ */


/* Function tools support macro */
/*
#define   _LG_TOOLS_
    #define  _LG_GB2312_TO_UNICODE_
    #define  _LG_UNICODE_TO_GB2312_
        #define  _LG_FAST_UNICODE_TO_GB2312_
*/

/* Optization storage support macro */
#define  GUI_VAR_CONST                                     const
#define  GUI_DATA_CONST                                    const

/* Optization inline keyword */
/*
#define  _LG_INLINE_
    #define  DEFAULT_INLINE                                inline
*/

/* Debug support macro */
/* #define  _LG_DEBUG_ */

#endif  /* __LGUI_CONFIG_BASIC_HEADER__ */
