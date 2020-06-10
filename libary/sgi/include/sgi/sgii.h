#ifndef __SGI_INTERFACE_H__ /* interface */
#define __SGI_INTERFACE_H__

/* SGI: Simple graphical interface */
#include <stddef.h>
#include "sgi.h"

bool __SGI_DisplayWindowHandleCheck(SGI_Display *display);
int __SGI_DisplayWindowHandleAdd(SGI_Display *display, unsigned int wid);
int __SGI_DisplayWindowHandleDel(SGI_Display *display, SGI_Window window);
int __SGI_DisplayWindowHandleFind(SGI_Display *display, SGI_Window window);

#define SGI_BAD_WIN_HANDLE(win) \
    (win < 0 || win >= SGI_WINDOW_HANDLE_NR)


#endif  /* __SGI_INTERFACE_H__ */