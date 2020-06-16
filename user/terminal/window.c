#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sgi/sgi.h>
#include "terminal.h"
#include "window.h"

#define DEBUG_LEVEL 1

int con_open_window()
{
    SGI_Display *display = SGI_OpenDisplay();
    
    if (display == NULL) {
        printf("[%s] %s: open display failed!\n", APP_NAME, __func__);
        return -1;
    }
#if DEBUG_LEVEL == 1
    printf("[%s] %s: open display %d ok.\n", APP_NAME, __func__, display->id);
#endif
    screen.display = display;

    SGI_Window win = SGI_CreateSimpleWindow(
        display,
        display->root_window,
        50,
        50,
        screen.width,
        screen.height,
        screen.background_color
    );
    if (win < 0) {
        printf("[%s] %s: create window failed!\n", APP_NAME, __func__);
        SGI_CloseDisplay(display);
        return -1;
    }
    screen.win = win;
#if DEBUG_LEVEL == 1
    printf("[%s] %s: create window %d success.\n", APP_NAME, __func__, win);
#endif
    SGI_SetWMName(display, win, APP_NAME);
    
    if (SGI_MapWindow(display, win) < 0) {
        printf("[%s] %s: map window failed!\n", APP_NAME, __func__);
        SGI_DestroyWindow(display, win);
        SGI_CloseDisplay(display);
        return -1;
    }
#if DEBUG_LEVEL == 1
    printf("[%s] %s: map window success.\n", APP_NAME, __func__);
#endif
    screen.font = SGI_LoadFont(display, "standard-8*16");

    if (SGI_UpdateWindow(display, win, 0, 0, screen.width, screen.height) < 0) {
        printf("[%s] %s: update window failed!\n", APP_NAME, __func__);
        SGI_UnmapWindow(display, win);
        SGI_DestroyWindow(display, win);
        SGI_CloseDisplay(display);
        return -1;
    }
    /* 选择接收的输入内容 */
    SGI_SelectInput(display, win, SGI_ButtonPressMask | SGI_ButtonRleaseMask |
        SGI_KeyPressMask | SGI_KeyRleaseMask);
    
    /* 绘制光标 */
    draw_cursor();

    return 0;
}

int con_close_window()
{
    SGI_Window win = screen.win;
    SGI_Display *display = screen.display;

    if (SGI_UnmapWindow(display, win) < 0) {
        printf("[%s] %s: unmap window failed!\n", APP_NAME, __func__);
    } else {
        printf("[%s] %s: unmap window success.\n", APP_NAME, __func__);
    }
    
    if (SGI_DestroyWindow(display, win) < 0) {
        printf("[%s] %s: destroy window failed!\n", APP_NAME, __func__);
    } else {
        printf("[%s] %s: destroy window success.\n", APP_NAME, __func__);
    }
    if (SGI_CloseDisplay(display) < 0) {
        printf("[%s] %s: close display failed!\n", APP_NAME, __func__);
    } else {
        printf("[%s] %s: close display success.\n", APP_NAME, __func__);
    }
    return 0;
}

int con_event_loop()
{
    SGI_Window win = screen.win;
    SGI_Display *display = screen.display;
    SGI_Event event;

    while (1) {
        if (SGI_NextEvent(display, &event))
            continue;
        switch (event.type)
        {
        case SGI_KEY:
            if (event.key.state == SGI_PRESSED) {
                printf(
                    "[%s] keyboard key pressed [%x, %c] modify %x.\n", 
                    APP_NAME,
                    event.key.keycode.code, 
                    event.key.keycode.code,
                    event.key.keycode.modify
                );
                if (event.key.keycode.code == SGIK_ENTER)
                    screen.outc('\n');
                else 
                    screen.outc(event.key.keycode.code);
            } else {
                printf(
                    "[%s] keyboard key released [%x, %c] modify %x.\n",
                    APP_NAME,
                    event.key.keycode.code, 
                    event.key.keycode.code,
                    event.key.keycode.modify
                );
            }
            break;
        case SGI_QUIT:
            printf("[%s] %s: handle quit event.\n", APP_NAME, __func__);
            goto loop_end;
            break;
        default:
            break;
        }
    }
loop_end:
    return 0;
}
