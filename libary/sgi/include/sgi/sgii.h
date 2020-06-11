#ifndef __SGI_INTERFACE_H__ /* interface */
#define __SGI_INTERFACE_H__

/* SGI: Simple graphical interface */
#include <stddef.h>
#include "sgi.h"

bool SGI_DisplayWindowInfoCheck(SGI_Display *display);
int SGI_DisplayWindowInfoAdd(SGI_Display *display, SGI_WindowInfo *winfo);
int SGI_DisplayWindowInfoDel(SGI_Display *display, SGI_Window window);

#define SGI_BAD_WIN_HANDLE(win) \
    (win < 0 || win >= SGI_WINDOW_HANDLE_NR)

#define SGI_DISPLAY_GET_WININFO(display, window) \
        (SGI_BAD_WIN_HANDLE(window) ? NULL : &(display)->winfo_table[window])

#endif  /* __SGI_INTERFACE_H__ */