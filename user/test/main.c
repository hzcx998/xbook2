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

    SGI_SetWMName(display, win, "new title, 123 abc #$");

    static SGI_Argb icon[5*5*4];
    int i;
    for (i = 0; i < 5*5; i++) {
        icon[i] = SGI_RGB(i * 20, i* 15, i* 5);
    }
    SGI_SetWMIcon(display, win, icon, 5, 5);

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


    SGI_Argb pixmap[10*10*sizeof(SGI_Argb)];
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 10; x++) {
            pixmap[y * 10 + x] = SGI_ARGB(0xff, x * 10, x * 15, y * 10);
        }
    }
    SGI_WindowDrawPixmap(display, win, 100, 200, 10, 10, pixmap);
    
    SGI_DrawString(display, win, 100, 50, "hello, text!\nabc\n\rdef", 30, SGIC_BLUE);

    if (SGI_SetFont(display, win, SGI_LoadFont(display, "standard-8*16")) < 0) {
        printf("[test] set font failed!\n");
    }

    SGI_DrawString(display, win, 100, 200, "hello, text!\nabc\n\rdef", 30, SGIC_BLUE);
    
    if (SGI_UpdateWindow(display, win, 0, 0, 320, 240))
        printf("[test] update window failed!\n");
    else
        printf("[test] update window success!\n");

    
    printf("[test] window handle %d\n", win);

    SGI_SelectInput(display, win, SGI_ButtonPressMask | SGI_ButtonRleaseMask |
        SGI_KeyPressMask | SGI_KeyRleaseMask | SGI_EnterWindow | SGI_LeaveWindow);

    SGI_Event event;
    SGI_Window event_window;
    while (1) {
        if (SGI_NextEvent(display, &event))
            continue;
        
        event_window = SGI_DISPLAY_EVENT_WINDOW(display);
        // printf("[test] event window %d\n", event_window);
        switch (event.type)
        {
        case SGI_MOUSE_BUTTON:
            if (event.button.state == SGI_PRESSED) {    // 按下
                if (event.button.button == 0) {
                    printf("[test] left button pressed.\n");
                } else if (event.button.button == 1) {
                    printf("[test] middle button pressed.\n");
                } else if (event.button.button == 2) {
                    printf("[test] right button pressed.\n");
                }
            } else {
                if (event.button.button == 0) {
                    printf("[test] left button released.\n");
                } else if (event.button.button == 1) {
                    printf("[test] middle button released.\n");
                } else if (event.button.button == 2) {
                    printf("[test] right button released.\n");
                }
            }
            break;
        case SGI_MOUSE_MOTION:
            if (event.motion.state == SGI_ENTER) {
                printf("[test] mouse enter window motion %d, %d.\n", event.motion.x, event.motion.y);
            } else if (event.motion.state == SGI_LEAVE) {
                printf("[test] mouse leave window motion %d, %d.\n", event.motion.x, event.motion.y);
            } else {
                printf("[test] mouse motion %d, %d.\n", event.motion.x, event.motion.y);
            }
            break;
        case SGI_KEY:
            if (event.key.state == SGI_PRESSED) {
                printf("[test] keyboard key pressed [%x, %c] modify %x.\n", event.key.keycode.code, 
                    event.key.keycode.code, event.key.keycode.modify);
                
                
            } else {
                printf("[test] keyboard key released [%x, %c] modify %x.\n", event.key.keycode.code, 
                    event.key.keycode.code, event.key.keycode.modify);
                if (event.key.keycode.code == SGIK_Q || event.key.keycode.code == SGIK_q) {
                    goto exit_gui;
                }
            }
            break;
        case SGI_QUIT:
            printf("[test] get quit event.\n");
            goto exit_gui;
            break;
        default:
            break;
        }
    }
    sleep(1);
exit_gui:
    
    if (SGI_UnmapWindow(display, win) < 0) {
        printf("[test] unmap window failed!\n");
    } else {
        printf("[test] unmap window success!\n");
    }

    if (SGI_DestroyWindow(display, win) < 0) {
        printf("[test] destroy window failed!\n");
    } else {
        printf("[test] destroy window success!\n");
    }
    
    SGI_CloseDisplay(display);
    printf("[test] close display ok!\n");

    return 0;
}
