#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>

#include <srv/guisrv.h>
#include <sys/srvcall.h>
#include <sys/proc.h>
#include <sgi/sgi.h>

int main(int argc, char *argv[])
{
    printf("hello, test!\n");
    sleep(1);

    SGI_Display *display = SGI_OpenDisplay();
    if (display == NULL) {
        printf("[test] open gui failed!\n");
        return -1;
    }
    printf("[test] open display ok!\n");

    SGI_Window win = SGI_CreateSimpleWindow(
        display,
        display->root_window,
        10,
        100,
        320,
        240,
        0XffFAFA55
    );

    if (win < 0) {
        printf("[test] create window failed!\n");
    }
    printf("[test] create window success!\n");

    if (SGI_MapWindow(display, win)) {
        printf("[test] map window failed!\n");
    } else {
        printf("[test] map window success!\n");
    }
    int x, y;
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 320; x++) {
            SGI_WindowDrawPixel(display, win, x, y, SGIC_RED);
        }
    }
    
    SGI_WindowDrawRect(display, win, 50, 50, 100, 50, SGIC_BLUE);
    SGI_WindowDrawRectFill(display, win, 70, 100, 50, 100, SGIC_GREEN);

    if (SGI_UpdateWindow(display, win, 0, 0, 320, 240))
        printf("[test] update window failed!\n");
    else
        printf("[test] update window success!\n");

    while (1)
    {
        /* code */
    }
    
    if (SGI_UnmapWindow(display, win)) {
        printf("[test] unmap window failed!\n");
    } else {
        printf("[test] unmap window success!\n");
    }

    if (SGI_DestroyWindow(display, win)) {
        printf("[test] destroy window failed!\n");
    } else {
        printf("[test] destroy window success!\n");
    }
    
    SGI_CloseDisplay(display);
    printf("[test] close display ok!\n");

    return 0;
}
