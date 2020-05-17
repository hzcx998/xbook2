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


#ifndef  __LGUI_TYPE_GUI_HEADER__
#define  __LGUI_TYPE_GUI_HEADER__

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif



/* NULL */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


/* Flag */
#ifndef  GUI_UNUSED
#define  GUI_UNUSED                  0
#endif
#ifndef  GUI_USED
#define  GUI_USED                    1
#endif


/* Color type */
#define  GUI_COLOR                   UINT32
#define  SCREEN_COLOR                UINT32


/* GUI macro */
#ifndef  GUI_MIN
#define  GUI_MIN(a,b)               ((a)<(b)?(a):(b))
#endif

#ifndef  GUI_MAX
#define  GUI_MAX(a,b)               ((a)>(b)?(a):(b))
#endif



/* Color group and color role */
#ifdef  _LG_WINDOW_

enum  _COLOR_GROUP
{
    DISABLED_GROUP = 0, INACTIVE_GROUP = 1, ACTIVE_GROUP = 2, MAX_COLOR_GROUP = ACTIVE_GROUP
};
typedef	enum  _COLOR_GROUP  COLOR_GROUP;

enum  _COLOR_ROLE
{
    NO0_ROLE = 0, NO1_ROLE = 1, NO2_ROLE = 2, NO3_ROLE = 3, NO4_ROLE = 4, NO5_ROLE = 5, MAX_COLOR_ROLE = NO5_ROLE,
    BACK_ROLE = NO0_ROLE, FORE_ROLE = NO1_ROLE, TEXT_BACK_ROLE = NO2_ROLE, TEXT_FORE_ROLE = NO3_ROLE
};
typedef	enum  _COLOR_ROLE  COLOR_ROLE;

#else    /* _LG_WINDOW_ */

enum  _COLOR_GROUP
{
    DISABLED_GROUP = 0, INACTIVE_GROUP = 0, ACTIVE_GROUP = 0, MAX_COLOR_GROUP = ACTIVE_GROUP
};
typedef	enum  _COLOR_GROUP  COLOR_GROUP;

enum  _COLOR_ROLE
{
    NO0_ROLE = 0, NO1_ROLE = 1, NO2_ROLE = 2, NO3_ROLE = 3, NO4_ROLE = 3, NO5_ROLE = 3, MAX_COLOR_ROLE = NO3_ROLE,
    BACK_ROLE = NO0_ROLE, FORE_ROLE = NO1_ROLE, TEXT_BACK_ROLE = NO2_ROLE, TEXT_FORE_ROLE = NO3_ROLE
};
typedef	enum  _COLOR_ROLE  COLOR_ROLE;

#endif



/* GUI_POINT */
struct  _GUI_POINT
{
    int  x;
    int  y;
};
typedef  struct _GUI_POINT   GUI_POINT;


/* GUI_DOUBLE_POINT */
struct  _GUI_DOUBLE_POINT
{
    DOUBLE  x;
    DOUBLE  y;
};
typedef  struct _GUI_DOUBLE_POINT   GUI_DOUBLE_POINT;



/* GUI_RECT */
struct  _GUI_RECT
{
    int  left;
    int  top;
    int  right;
    int  bottom;
};
typedef  struct _GUI_RECT   GUI_RECT;

#define  GUI_RECTW(rect)    (((rect)->right)-((rect)->left)+1)
#define  GUI_RECTH(rect)    (((rect)->bottom)-((rect)->top)+1)

/* GUI_DOUBLE_RECT */
struct  _GUI_DOUBLE_RECT
{
    DOUBLE  left;
    DOUBLE  top;
    DOUBLE  right;
    DOUBLE  bottom;
};
typedef  struct _GUI_DOUBLE_RECT   GUI_DOUBLE_RECT;




/* GUI_SIZE */
struct  _GUI_SIZE
{
    int  cx;
    int  cy;
};
typedef  struct _GUI_SIZE   GUI_SIZE;


struct tagGUI_FLOAT_RGB 
{
    FLOAT   blue;
    FLOAT   green;
    FLOAT   red;
    FLOAT   appha;
};
typedef  struct  tagGUI_DOUBLE_RGB  GUI_DOUBLE_RGB; 


struct tagGUI_DOUBLE_RGB 
{
    DOUBLE  blue;
    DOUBLE  green;
    DOUBLE  red;
    DOUBLE  alpha;
};
typedef  struct  tagGUI_DOUBLE_RGB  GUI_DOUBLE_RGB; 



/* GUI_ROTATE */
struct  _GUI_ROTATE
{
    int  is_rotate;
    int  x;
    int  y;
    int  theta;

    GUI_DOUBLE_RGB  *buffer;
    unsigned  int    buf_size;
};
typedef  struct _GUI_ROTATE   GUI_ROTATE;


/* GUI_TRANSFORM */
struct  _GUI_TRANSFORM
{
    int                is_transform;
    GUI_DOUBLE_POINT   corner[4];

    GUI_DOUBLE_POINT  *raw_buffer;
    unsigned  int      raw_size;

    GUI_DOUBLE_RGB    *transform_buffer;
    unsigned  int      transform_size;
 
    GUI_DOUBLE_POINT   offset;
};
typedef  struct _GUI_TRANSFORM   GUI_TRANSFORM;



/* GUI_SYMMETRY */
#define  GUI_SYMMETRY_VLINE               0
#define  GUI_SYMMETRY_HLINE               1
#define  GUI_SYMMETRY_LINE                2
#define  GUI_SYMMETRY_POINT               3

/* GUI_SYMMETRY */
struct  _GUI_SYMMETRY
{
    int   is_symmetry;
    int   symmetry_type;
    GUI_POINT  point[2];
};
typedef  struct _GUI_SYMMETRY   GUI_SYMMETRY;


/* GUI_SYMMETRY_ROTATE */
#define  GUI_FIRST_SYMMETRY           0
#define  GUI_FIRST_ROTATE             1


struct  _GUI_SYMMETRY_ROTATE
{
    int             first;	
    GUI_SYMMETRY    symmetry;
    GUI_ROTATE      rotate;
};
typedef  struct _GUI_SYMMETRY_ROTATE   GUI_SYMMETRY_ROTATE;



/* GUI_PEN */
#define  SOLID_PEN_TYPE               0
#define  DASH_PEN_TYPE                1
#define  DOT_PEN_TYPE                 2
#define  DASHDOT_PEN_TYPE             3
#define  DASHDOTDOT_PEN_TYPE          4

struct  _GUI_PEN
{
    BINT        type;
    BINT        width;
    GUI_POINT   cur_pos;
};
typedef  struct _GUI_PEN   GUI_PEN;


/* Brush */
/* Brush type */
#define  SOLID_BRUSH_TYPE            (0)

/* Hatch type */
#define  HORIZONTAL_HATCH_TYPE        0       /* ----- */
#define  VERTICAL_HATCH_TYPE          1       /* ||||| */
#define  FDIAGONAL_HATCH_TYPE         2       /* \\\\\ */
#define  BDIAGONAL_HATCH_TYPE         3       /* ///// */
#define  CROSS_HATCH_TYPE             4       /* +++++ */
#define  DIAGCROSS_HATCH_TYPE         5       /* xxxxx */

struct  _GUI_BRUSH
{
    BINT        brush_type;
    BINT        hatch_type;
    GUI_POINT   cur_pos; 
};
typedef  struct _GUI_BRUSH   GUI_BRUSH;


/* GUI_TEXT_METRIC */
struct  _GUI_TEXT_METRIC
{
    int    space_left;
    int    space_top;
    int    space_right;
    int    space_bottom;

    UCHAR  is_strike_out;
    UCHAR  frame_style;
    char   offset_italic;
    char   offset_escapement;
};
typedef  struct _GUI_TEXT_METRIC	GUI_TEXT_METRIC;


/* GUI_FONT */

#define  LGUI_TO_STRING(s)                         s

/* Define character glyph */
#define  DECLARE_MONO_DISCRETE_UCHAR2(data_size)   \
struct _MONO_DISCRETE_CHAR_##data_size             \
{                                                  \
    UCHAR  code[2];                                \
    UCHAR  data[LGUI_TO_STRING(data_size)];        \
};                                                 \
typedef	struct _MONO_DISCRETE_CHAR_##data_size  MONO_DISCRETE_CHAR_##data_size

#define  DECLARE_MONO_DISCRETE_UINT16(data_size)   \
struct _MONO_DISCRETE_CHAR_##data_size             \
{                                                  \
    UINT16  code;                                  \
    UCHAR   data[LGUI_TO_STRING(data_size)];       \
};                                                 \
typedef	struct _MONO_DISCRETE_CHAR_##data_size  MONO_DISCRETE_CHAR_##data_size


struct _MIXED_CHARSET_CHAR 
{
    UCHAR  width;
    UCHAR  height;
    UCHAR  *data;
};
typedef	struct	_MIXED_CHARSET_CHAR  MIXED_CHARSET_CHAR;


struct _MIXED_DISCRETE_UINT16 
{
    UINT16  code;
    UCHAR   width;
    UCHAR   height;
    UCHAR   *data;
};
typedef	struct _MIXED_DISCRETE_UINT16  MIXED_DISCRETE_UINT16;

struct _MIXED_DISCRETE_UCHAR2 
{
    UCHAR   code[2];
    UCHAR   width;
    UCHAR   height;
    UCHAR   *data;
};
typedef	struct _MIXED_DISCRETE_UCHAR2  MIXED_DISCRETE_UCHAR2;


/* Define font item */
struct _MONO_CHARSET_FONT 
{
    UCHAR                       name[FONT_NAME_LEN];
    #ifdef  _LG_FONT_ID_
    UCHAR                       id;
    #endif
    int                         (*is_in_this_charset_block)(const TCHAR *code);

    unsigned int                is_get_serial_data;
    unsigned int                (*get_serial_data)(const TCHAR *code, UCHAR *data, unsigned int *data_len);

    unsigned int                (*get_data_start_index)(const TCHAR *code);

    UCHAR                       width;
    UCHAR                       height;
    void                       *data;
    struct _MONO_CHARSET_FONT  *next;
};
typedef	struct _MONO_CHARSET_FONT  MONO_CHARSET_FONT;


struct _MONO_DISCRETE_FONT 
{
    UCHAR                        name[FONT_NAME_LEN];
    #ifdef  _LG_FONT_ID_
    UCHAR                        id;
    #endif
    UCHAR                        width;
    UCHAR                        height;
    UINT                         counter;
    UCHAR                       *mono_char;
    struct _MONO_DISCRETE_FONT  *next;
};
typedef	struct _MONO_DISCRETE_FONT  MONO_DISCRETE_FONT;


struct _MIXED_CHARSET_FONT 
{
    UCHAR                       name[FONT_NAME_LEN];
    #ifdef  _LG_FONT_ID_
    UCHAR                       id;
    #endif
    int                         (*is_in_this_charset_block)(const TCHAR *code);
    unsigned int                (*get_code_index)(const TCHAR *code);
    MIXED_CHARSET_CHAR          *mixed_char;
    struct _MIXED_CHARSET_FONT  *next;
};
typedef	struct	_MIXED_CHARSET_FONT  MIXED_CHARSET_FONT;



/* MIXED_DISCRETE_FONT char_type define */
#define  MIXED_DISCRETE_UINT16_TYPE        0
#define  MIXED_DISCRETE_UCHAR2_TYPE        1

struct _MIXED_DISCRETE_FONT 
{
    UCHAR                         name[FONT_NAME_LEN];
    #ifdef  _LG_FONT_ID_
    UCHAR                         id;
    #endif
    UCHAR                         type;
    UINT                          counter;
    void                         *list;
    struct _MIXED_DISCRETE_FONT  *next;
};
typedef	struct _MIXED_DISCRETE_FONT  MIXED_DISCRETE_FONT;


enum  _FONT_TYPE 
{
    MONO_CHARSET_FONT_TYPE = 0,
    MONO_DISCRETE_FONT_TYPE,
    MIXED_CHARSET_FONT_TYPE,
    MIXED_DISCRETE_FONT_TYPE
};
typedef	enum  _FONT_TYPE  FONT_TYPE;

#define   MIN_FONT_TYPE        MONO_CHARSET_FONT_TYPE
#define   MAX_FONT_TYPE        MIXED_DISCRETE_FONT_TYPE


struct	_GUI_FONT
{
    FONT_TYPE          type;
    void              *font;
    struct _GUI_FONT  *next;
};
typedef	struct	_GUI_FONT  GUI_FONT;


struct  _GUI_DC
{
    BUINT        type;
    BUINT        used;

    BUINT        back_mode;

    BUINT        cur_group;

    GUI_COLOR    color[MAX_COLOR_GROUP+1][MAX_COLOR_ROLE+1];
    #ifdef  _LG_ALPHA_BLEND_
    BUINT        is_alpha_blend;
    BUINT        alpha_blend_op_mode;
    #endif

    #ifdef  _LG_FONT_
    GUI_FONT    *font;
    #endif

    #ifdef  _LG_TEXT_METRIC_
    GUI_TEXT_METRIC  text_metric;
    #endif

    #ifdef  _LG_PEN_
    /* Relative coordinate */
    GUI_PEN      pen;
    #endif

    #ifdef  _LG_BRUSH_
    /* Relative coordinate */
    GUI_BRUSH    brush;
    #endif

    /* Absolute coordinate */
    GUI_RECT     rect;

    /* Absolute coordinate */
    BUINT        is_paint_rect;
    GUI_RECT     paint_rect;
   

    #ifdef  _LG_WINDOW_
    void        *hwnd;
    #endif
};
typedef  struct _GUI_DC    GUI_DC;
typedef  struct _GUI_DC   *HDC;



/* GUI_BANK */
struct  _GUI_BANK
{
    char         *p;
    unsigned int  len;
    unsigned int  width;
    unsigned int  height;
    unsigned int  bits;
    unsigned int  is_transparent;
    SCREEN_COLOR  transparent_color;
    unsigned int  option;
};
typedef  struct _GUI_BANK   GUI_BANK;



#endif  /* __LGUI_TYYPE_GUI_HEADER__ */
