#ifndef __SGI_H__
#define __SGI_H__

/* SGI: Simple graphical interface */
#include <stddef.h>
#include "sgiw.h"
#include "sgie.h"

/* 支持打开的最大窗口句柄数 */
#define SGI_WINDOW_HANDLE_NR   8

typedef struct _SGI_Display
{
    unsigned int connected;         /* 连接标志：是否连接上图形服务 */
    unsigned int width;             /* 窗口宽度 */
    unsigned int height;            /* 窗口高度 */
    SGI_Window root_window;         /* 根窗口 */
    SGI_WindowInfo winfo_table[SGI_WINDOW_HANDLE_NR]; /* 窗口信息表 */
    int event_msgid;                /* 消息队列：用来接收事件 */
    int request_msgid;              /* 消息队列：用来请求事件 */
    unsigned int id;                /* 显示id */
    SGI_Window event_window;        /* 接收事件的窗口 */
} SGI_Display;

#define SGI_DISPLAY_EVENT_WINDOW(display) \
        (display)->event_window

void *SGI_Malloc(size_t size);
void SGI_Free(void *ptr);

SGI_Display *SGI_OpenDisplay();
int SGI_CloseDisplay(SGI_Display *display);

SGI_Window SGI_CreateSimpleWindow(
    SGI_Display *display,
    SGI_Window parent,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    unsigned int background
);
int SGI_DestroyWindow(SGI_Display *display, SGI_Window window);

int SGI_MapWindow(SGI_Display *display, SGI_Window window);
int SGI_UnmapWindow(SGI_Display *display, SGI_Window window);

int SGI_UpdateWindow(
    SGI_Display *display,
    SGI_Window window,
    int left,
    int top,
    int right,
    int bottom
);

int SGI_WindowDrawPixel(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    SGI_Argb color
);

int SGI_WindowDrawRectFill(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    
    SGI_Argb color
);
int SGI_WindowDrawRect(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    SGI_Argb color
);

int SGI_WindowDrawPixmap(
    SGI_Display *display,
    SGI_Window window,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    SGI_Argb *pixmap
);

int SGI_NextEvent(SGI_Display *display, SGI_Event *event);
int SGI_PollEvent(SGI_Display *display, SGI_Event *event);

int SGI_SetWMName(
    SGI_Display *display,
    SGI_Window window,
    char *name
);
int SGI_SetWMIconName(
    SGI_Display *display,
    SGI_Window window,
    char *name
);

int SGI_SetWMIcon(
    SGI_Display *display,
    SGI_Window window,
    SGI_Argb *pixmap,
    unsigned int width,
    unsigned int height
);

int SGI_SelectInput(
    SGI_Display *display,
    SGI_Window w,
    long mask
);

#endif  /* __SGI_H__ */