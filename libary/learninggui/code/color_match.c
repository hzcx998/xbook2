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


#include  <color_match.h>


#define S(x)           ((x)*(x))
#define SQUARES(base)  S(base+0),  S(base+1),  S(base+2),  S(base+3), \
                       S(base+4),  S(base+5),  S(base+6),  S(base+7), \
                       S(base+8),  S(base+9), S(base+10), S(base+11), \
                       S(base+12), S(base+13), S(base+14), S(base+15)

static  GUI_VAR_CONST  UINT16  square_array[] = 
{
   SQUARES(0*16),  SQUARES(1*16),  SQUARES(2*16),  SQUARES(3*16),
   SQUARES(4*16),  SQUARES(5*16),  SQUARES(6*16),  SQUARES(7*16),
   SQUARES(8*16),  SQUARES(9*16),  SQUARES(10*16), SQUARES(11*16),
   SQUARES(12*16), SQUARES(13*16), SQUARES(14*16), SQUARES(15*16)
};
  
#define  SQUARE(dist)  square_array[dist]



UINT32  COLOR_MATCH_INDEX(GUI_COLOR color0, GUI_COLOR color1)
{
    INT16   dist = 0;
    UINT32  sum  = 0;

    dist  = (color0&0xFF) - (color1&0xFF);
    if (dist < 0)
        dist = -dist;
    sum = SQUARE(dist);

    dist  = ((color0>>8)&0xFF) - ((color1>>8)&0xFF);
    if (dist < 0)
        dist = -dist;
    sum += SQUARE(dist);

    dist  = ((color0>>16)&0xFF) - ((color1>>16)&0xFF);
    if (dist < 0)
        dist = -dist;

    return  (sum + SQUARE(dist));
}
