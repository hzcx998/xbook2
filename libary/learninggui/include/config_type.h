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

#ifndef	 __LGUI_CONFIG_TYPE_HEADER__
#define  __LGUI_CONFIG_TYPE_HEADER__

/* User type definition */
#ifndef  INT8
typedef  char                  INT8;
#endif
#ifndef  INT16
typedef  short int             INT16;
#endif
#ifndef  INT32
typedef  int                   INT32;
#endif
#ifndef  INT64
typedef  long long             INT64;
#endif


#ifndef  UINT8
typedef  unsigned char         UINT8;
#endif
#ifndef  UINT16
typedef  unsigned short int    UINT16;
#endif
#ifndef  UINT32
typedef  unsigned int          UINT32;
#endif
#ifndef  UINT64
typedef  unsigned long long    UINT64;
#endif

#ifndef  UINT
typedef  unsigned int          UINT;
#endif

#ifndef  UCHAR
typedef  unsigned char         UCHAR;
#endif


/* Sram or ram access bus width. It can be more than real bus width, but can not be less than real bus width. */
#ifndef  BINT
typedef  int                   BINT;
#endif
#ifndef  BUINT
typedef  unsigned int          BUINT;
#endif

#endif	/* __LGUI_CONFIG_TYPE_HEADER__ */
