#ifndef _GAPI_COLOR_H
#define _GAPI_COLOR_H

#define GC_NO_ALPHA   255

/**
 * argb color
 * 
 * blue: 0~7
 * green: 8~15
 * red: 16~23
 * alpha: 24~31
 */
#define _GC_ARGB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) 
#define GC_ARGB(a, r, g, b) _GC_ARGB((a) & 0xff, (r)  & 0xff, (g) & 0xff, (b) & 0xff)
#define GC_RGB(r, g, b) GC_ARGB(GC_NO_ALPHA, r, g, b)

/* 常用颜色 */
#define GC_RED        GC_RGB(255, 0, 0)
#define GC_GREEN      GC_RGB(0, 255, 0)
#define GC_BLUE       GC_RGB(0, 0, 255)
#define GC_WHITE      GC_RGB(255, 255, 255)
#define GC_BLACK      GC_RGB(0, 0, 0)
#define GC_GRAY       GC_RGB(195, 195, 195)
#define GC_LEAD       GC_RGB(127, 127, 127)
#define GC_YELLOW     GC_RGB(255, 255, 0)
#define GC_NONE       GC_ARGB(0, 0, 0, 0)

/* gui color type */
typedef unsigned int g_color_t;

#endif /* _GAPI_COLOR_H */
