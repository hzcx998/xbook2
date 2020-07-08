#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sgi/sgi.h>
#include "terminal.h"
#include "window.h"
#include "cmd.h"
#include "cursor.h"
#include "console.h"
#include "clipboard.h"
#include <sys/trigger.h>

#define DEBUG_LEVEL 0

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
        SGI_KeyPressMask | SGI_KeyRleaseMask | SGI_PointerMotionMask);
    
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
    SGI_Display *display = screen.display;
    SGI_Event event;

    cmdman->cmd_pos = cmdman->cmd_line;
    cmdman->cmd_len = 0;

    int j;
    char *q;
    int cx, cy;

    while ((cmdman->cmd_pos - buf) < count) {
        if (SGI_NextEvent(display, &event))
            continue;
        switch (event.type)
        {
        case SGI_MOUSE_BUTTON:
            if (event.button.state == SGI_PRESSED) {    // 按下
                if (event.button.button == 0) {
                    printf("[%s] left button pressed.\n", APP_NAME);
                    clipboard_start_select(event.button.x, event.button.y);

                } else if (event.button.button == 2) {
                    clipboard_copy_select();
                } 
            } else {
                if (event.button.button == 0) {
                    printf("[%s] left button released.\n", APP_NAME);
                    clipboard_end_select(event.button.x, event.button.y);

                }
            }
            break;
        case SGI_MOUSE_MOTION:
            clipboard_move_select(event.motion.x, event.motion.y);
            break;
        case SGI_KEY:
            if (event.key.state == SGI_PRESSED) {
                /* 组合按键 */
                if (event.key.keycode.modify & SGI_KMOD_CTRL) {
                    if (event.key.keycode.code == SGIK_UP) {
                        scroll_screen(CON_SCROLL_UP, 1, 0, 1);
                        //print_cursor();
                        break;
                    } else if (event.key.keycode.code == SGIK_DOWN) {
                        scroll_screen(CON_SCROLL_DOWN, 1, 0, 1);
                        //print_cursor();
                        break;
                    }
                }
                /* 过滤一些按键 */
                switch (event.key.keycode.code) {
                case SGIK_UP:
                    clipboard_break_select();
                    focus_cursor();
                    /* 选择上一个的命令 */
                    //move_cursor_off(0, -1);
                    cmd_buf_select(-1);
                    break;
                case SGIK_DOWN:
                    clipboard_break_select();
                    focus_cursor();
                    /* 选择下一个命令 */
                    //move_cursor_off(0, 1);
                    cmd_buf_select(1);
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
                    clipboard_break_select();
                    focus_cursor();
                    if(cmdman->cmd_pos > buf){
                        --cmdman->cmd_pos;
                        move_cursor_off(-1, 0);
                    }
                    break;
                case SGIK_RIGHT:
                    clipboard_break_select();
                    focus_cursor();
                    if ((cmdman->cmd_pos - buf) < cmdman->cmd_len) {
                        move_cursor_off(1, 0);
                        ++cmdman->cmd_pos;
                    }
                    break;
                case SGIK_ENTER:
                    clipboard_break_select();
                    focus_cursor();
                    move_cursor_off(cmdman->cmd_len - (cmdman->cmd_pos - buf), 0);
                    screen.outc('\n');
                    buf[cmdman->cmd_len] = 0;
                    /* 发送给命令行 */
                    return 0;   /* 执行命令 */
                case SGIK_BACKSPACE:
                    clipboard_break_select();
                    focus_cursor();
                    if(cmdman->cmd_pos > buf){
                        if (cmdman->cmd_pos >= buf + cmdman->cmd_len) { /* 在末尾 */
                            --cmdman->cmd_pos;
                            *cmdman->cmd_pos = '\0';
                            screen.outc('\b');
                      
                        } else {    /* 在中间 */
                            --cmdman->cmd_pos;
                            /* 获取现在的位置 */
                            get_cursor(&cx, &cy);
                            /* 去除后面的字符 */
                            j = strlen(cmdman->cmd_pos);
                            while (j--)
                                screen.outc(' ');
                            /* 移动回到原来的位置 */
                            move_cursor(cx, cy);

                            for (q = cmdman->cmd_pos; q < buf + cmdman->cmd_len; q++) {
                                *q = *(q + 1);
                            }
                            /* 向前移动一个位置 */
                            move_cursor_off(-1, 0);
                            /* 获取现在的位置 */
                            get_cursor(&cx, &cy);
                            screen.outs(cmdman->cmd_pos);
                            /* 移动回到原来的位置 */
                            move_cursor(cx, cy);
                        }
                        cmdman->cmd_len--;
                    }
                    break;
                default:
                    clipboard_break_select();
                    focus_cursor();
                    /* 取消框选 */

                    if (cmdman->cmd_pos >= buf + cmdman->cmd_len) { /* 在末尾 */
                        *cmdman->cmd_pos = event.key.keycode.code;
  
                        screen.outc(event.key.keycode.code);
                        
                    } else { 
                        /* 把后面的数据向后移动一个单位 ab1c|def */
                        for (q = buf + cmdman->cmd_len; q > cmdman->cmd_pos; q--) {
                            *q = *(q - 1);
                            *(q - 1) = '\0';
                        }
                        
                        get_cursor(&cx, &cy);

                        /* 发送给命令行 */
                        *cmdman->cmd_pos = event.key.keycode.code;
                        screen.outs(cmdman->cmd_pos);
                        move_cursor(cx + 1, cy);
                    }
                    cmdman->cmd_pos++;
                    cmdman->cmd_len++;
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


int con_event_poll(char *buf, int pid)
{
    SGI_Display *display = screen.display;
    SGI_Event event;

    *buf = 0;

    if (SGI_PollEvent(display, &event))
        return -1;

    switch (event.type)
    {
    case SGI_KEY:
        /* 只捕捉按下 */
        if (event.key.state == SGI_PRESSED) {
            *buf = event.key.keycode.code;
            /* 组合按键 */
            if (event.key.keycode.modify & SGI_KMOD_CTRL) {
                /* ctrl + c -> 打断进程 */
                if (event.key.keycode.code == SGIK_C || event.key.keycode.code == SGIK_c) {
                    /* 激活pid的重软件触发器 */
                    triggeron(TRIGLSOFT, pid);
                    break;
                }
            }
            switch (*buf)
            {
            case SGIK_ENTER:
                *buf = '\n';
                break;
            default:
                break;
            }
        } else {
            return -1;
        }
        break;
    case SGI_QUIT:
        printf("[%s] %s: handle quit event.\n", APP_NAME, __func__);
        return -1;  /* 退出 */
    default:
        return -1;
    }

    return 0;
}
