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

#include  <lock.h>

#include  <color_match.h>
#include  <palette.h>
#include  <palette_conversion.h>


#ifdef  _LG_COLOR_CONVERSION_
#ifdef  _LG_PALETTE_CONVERSION_

SCREEN_COLOR  in_gui_to_palette_index(GUI_COLOR gui_color)
{
    UINT32 diff      = 0;
    UINT32 best_diff = 0x00FFFFFF;

    unsigned int best_index = 0;
    unsigned int i          = 0;


    for ( i = 0; i < (lpal.num); i++ ) 
    {
        if ( lpal.entries[i] == gui_color ) 
            return  i;
    }

    for ( i = 0; i < (lpal.num); i++ ) 
    {
        diff = COLOR_MATCH_INDEX(gui_color, lpal.entries[i]);
        if ( diff < best_diff ) 
        {
            best_diff  = diff;
            best_index = i;
        }
    }

    return  best_index;
}

#ifndef  _LG_ALONE_VERSION_
SCREEN_COLOR  gui_to_palette_index(GUI_COLOR gui_color)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_to_palette_index(gui_color);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

GUI_COLOR  in_palette_index_to_gui(SCREEN_COLOR screen_color)
{    
    if ( screen_color >= (lpal.num) )
        return  0;
 
    return  lpal.entries[screen_color];
}

#ifndef  _LG_ALONE_VERSION_
GUI_COLOR  palette_index_to_gui(SCREEN_COLOR screen_color)
{
    int  ret = 0;

    gui_lock( );
    ret = in_palette_index_to_gui(screen_color);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_PALETTE_CONVERSION_ */
#endif  /* _LG_COLOR_CONVERSION_ */
