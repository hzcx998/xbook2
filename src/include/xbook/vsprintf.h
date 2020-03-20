/*
 * file:		include/lib/vsprintf.h
 * auther:		Jason Hu
 * time:		2019/6/2
 * copyright:	(C) 2018-2020 by Book OS developers. All rights reserved.
 */

#ifndef _LIB_VSPRINTF_H
#define _LIB_VSPRINTF_H

#include "stdarg.h"

#define STR_DEFAULT_LEN 256

int vsprintf(char *buf, const char *fmt, va_list args);
int vsnprintf(char *buf, int buflen, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int snprintf(char *buf, int buflen, const char *fmt, ...);

#endif  /* _LIB_VSPRINTF_H */
