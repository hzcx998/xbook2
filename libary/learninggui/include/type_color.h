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

#ifndef  __LGUI_TYPE_COLOR_HEADER__
#define  __LGUI_TYPE_COLOR_HEADER__

/*
 * BB means blue color component
 * GG means green color component
 * RR means red color component
 * AA means alpha color component
 *
 * GUI_COLOR format:                 0xAARRGGBB
 * The first byte(the lowest byte):  0xBB
 * The second byte:                  0xGG
 * The thirdth byte:                 0xRR
 * The fourth byte:                  0xAA
 *
 * SCREEN_COLOR: physical color(or palette index) for screen 
 */

/* LearningGUI system predefined GUI_COLOR */
#define  GUI_RED                            0x00FF0000
#define  GUI_GREEN                          0x0000FF00
#define  GUI_BLUE                           0x000000FF

#define  GUI_BLACK                          0x00000000
#define  GUI_LIGHT_BLACK                    0x00101010

#define  GUI_HEAVY_DARK                     0x00202020
#define  GUI_DARK                           0x00404040
#define  GUI_LIGHT_DARK                     0x00606060


#define  GUI_GRAY                           0x00808080
#define  GUI_LIGHT_GRAY                     0x00C0C0C0

#define  GUI_YELLOW                         0x00FFFF00

#define  GUI_BROWN                          0x00A52A2A
#define  GUI_MAGENTA                        0x008B008B
#define  GUI_CYAN                           0x0000FFFF

#define  GUI_LIGHT_GREEN                    0x0080FF80
#define  GUI_LIGHT_RED                      0x00FF8080
#define  GUI_LIGHT_CYAN                     0x00FFFF80
#define  GUI_LIGHT_MAGENTA                  0x00FF80FF
#define  GUI_LIGHT_BLUE                     0x008080FF

#define  GUI_LIGHT_WHITE                    0x00E0E0E0
#define  GUI_WHITE                          0x00FFFFFF


/* User color definition macro */
#define  GUI_ARGB(alpha,red,green,blue)     ((((alpha)*0xFF)<<24)|(((red)&0xFF)<<16)|(((green)&0xFF)<<8)|((blue)&0xFF))

/* Default  color */
#define  GUI_DEFAULT_BCOLOR                 GUI_BLUE
#define  GUI_DEFAULT_FCOLOR                 GUI_WHITE
#define  GUI_DEFAULT_TEXT_BCOLOR            GUI_BLUE
#define  GUI_DEFAULT_TEXT_FCOLOR            GUI_WHITE

/* 16-bit color format */
#define  GUI_COLOR_565_FORMAT               0
#define  GUI_COLOR_555_FORMAT               1

#endif  /* __LGUI_TYPE_COLOR_HEADER__ */
