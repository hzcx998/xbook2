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

#include  <hz1616_punctuation.h>

      
#ifdef  _LG_UNICODE_VERSION_
#ifdef  _LG_HZ1616_PUNCTUATION_

DECLARE_MONO_DISCRETE_UINT16(32);

static GUI_DATA_CONST  MONO_DISCRETE_CHAR_32  hz1616_punctuation[] = 
{
    {
        /*£¬*/
        0xFF0C,
        {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x18,0x00,0x18,0x00,0x10,0x00,0x20,0x00,0x00,0x00,0x00,0x00
        }
    },
    {
        /*¡£*/
        0x3002,
        {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x48,0x00,0x48,0x00,0x30,0x00,0x00,0x00
        }
    }
};

GUI_VAR_CONST  MONO_DISCRETE_FONT  lhzp16 = {
    /* .name      = */ "HZP16",
    #ifdef  _LG_FONT_ID_
    /* .id        = */ 32,
    #endif
    /* .width     = */ 16,
    /* .height    = */ 16,
    /* .counter   = */ sizeof(hz1616_punctuation)/sizeof(MONO_DISCRETE_CHAR_32),
    /* .mono_char = */ (UCHAR *)hz1616_punctuation, 
    /* .next      = */ 0
};

#endif  /* _LG_HZ1616_PUNCTUATION_ */
#endif  /* _LG_UNICODE_VERSION_ */
