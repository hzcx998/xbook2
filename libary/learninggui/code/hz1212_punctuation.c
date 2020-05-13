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

#include  <hz1212_punctuation.h>

   
#ifdef  _LG_UNICODE_VERSION_
#ifdef  _LG_HZ1212_PUNCTUATION_

DECLARE_MONO_DISCRETE_UINT16(18);

static GUI_DATA_CONST  MONO_DISCRETE_CHAR_18  hz1212_punctuation[] = 
{
    {
        /*£¬*/
        0xFF0C,
        {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x01,0x80,0x18,0x01,0x80,0x30,0x00,0x00
        }
    },
    {
        /*¡£*/
        0x3002,
        {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x06,0x00,0x90,0x09,0x00,0x60,0x00,0x00
        }
    }
};

GUI_VAR_CONST  MONO_DISCRETE_FONT  lhzp12 = {
    /* .name      = */ "HZP12",
    #ifdef  _LG_FONT_ID_
    /* .id        = */ 33,
    #endif
    /* .width     = */ 12,
    /* .height    = */ 12,
    /* .counter   = */ sizeof(hz1212_punctuation)/sizeof(MONO_DISCRETE_CHAR_18),
    /* .mono_char = */ (UCHAR *)hz1212_punctuation, 
    /* .next      = */ 0
};

#endif  /* _LG_HZ1212_PUNCTUATION_ */
#endif  /* _LG_UNICODE_VERSION_ */
