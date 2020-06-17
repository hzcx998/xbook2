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

int con_event_loop(char *buf, int count)
{
    SGI_Window win = screen.win;
    SGI_Display *display = screen.display;
    SGI_Event event;
    char *pos = buf;
    int len = 0;    /* 命令字符长度 */
    int i, j;
    char *q;

    int cx, cy;
    while ((pos - buf) < count) {
        if (SGI_NextEvent(display, &event))
            continue;
        switch (event.type)
        {
        case SGI_KEY:
            if (event.key.state == SGI_PRESSED) {
                /* 组合按键 */
                if (event.key.keycode.modify & SGI_KMOD_CTRL) {
                    if (event.key.keycode.code == SGIK_UP) {
                        scroll_screen(CON_SCROLL_UP, 1, 0, 1);
                        break;
                    } else if (event.key.keycode.code == SGIK_DOWN) {
                        scroll_screen(CON_SCROLL_DOWN, 1, 0, 1);
                        break;
                    }
                }
                /* 过滤一些按键 */
                switch (event.key.keycode.code)
                {
               
                case SGIK_UP:
                    /* 选择上一个的命令 */
                    move_cursor_off(0, -1);
                    break;
                case SGIK_DOWN:
                    /* 选择下一个命令 */
                    move_cursor_off(0, 1);
                    break;
                case SGIK_NUMLOCK:
                case SGIK_CAPSLOCK:
                case SGIK_SCROLLOCK:
                case SGIK_RSHIFT:
                case SGIK_LSHIFT:
                case SGIK_RCTRL:
                case SGIK_LCTRL:
                case SGIK_RALT:
                case SGIK_LALT:
                    break;
                case SGIK_LEFT:
                    if(pos > buf){
                        --pos;
                        move_cursor_off(-1, 0);
                    }
                    break;
                case SGIK_RIGHT:
                    if ((pos - buf) < len) {
                        move_cursor_off(1, 0);
                        ++pos;
                    }
                    break;
                case SGIK_ENTER:
                    move_cursor_off(len - (pos - buf), 0);
                    screen.outc('\n');
                    buf[len] = 0;
                    /* 发送给命令行 */
                    return 0;   /* 执行命令 */
                case SGIK_BACKSPACE:
                    if(pos > buf){
                        if (pos >= buf + len) { /* 在末尾 */
                            --pos;
                            *pos = '\0';
                            screen.outc('\b');
                      
                        } else {    /* 在中间 */
                            --pos;
                            /* 获取现在的位置 */
                            get_cursor(&cx, &cy);
                            /* 去除后面的字符 */
                            j = strlen(pos);
                            while (j--)
                                screen.outc(' ');
                            /* 移动回到原来的位置 */
                            move_cursor(cx, cy);

                            for (q = pos; q < buf + len; q++) {
                                *q = *(q + 1);
                            }
                            /* 向前移动一个位置 */
                            move_cursor_off(-1, 0);
                            /* 获取现在的位置 */
                            get_cursor(&cx, &cy);
                            screen.outs(pos);
                            /* 移动回到原来的位置 */
                            move_cursor(cx, cy);
                        }
                        len--;
                    }
                    break;
                default:
                    if (pos >= buf + len) { /* 在末尾 */
                        *pos = event.key.keycode.code;
                        screen.outc(event.key.keycode.code);
                    } else { 
                        /* 把后面的数据向后移动一个单位 ab1c|def */
                        for (q = buf + len; q > pos; q--) {
                            *q = *(q - 1);
                            *(q - 1) = '\0';
                        }
                        
                        get_cursor(&cx, &cy);

                        /* 发送给命令行 */
                        *pos = event.key.keycode.code;
                        screen.outs(pos);
                        move_cursor(cx + 1, cy);
                    }
                    pos++;
                    len++;
                    break;
                }
            }
            break;
        case SGI_QUIT:
            printf("[%s] %s: handle quit event.\n", APP_NAME, __func__);
            return -1;  /* 退出 */
        default:
            break;
        }
    }
    return 0;
}



