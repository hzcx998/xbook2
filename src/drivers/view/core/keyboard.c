#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/input.h>
#include "drivers/view/hal.h"
#include "drivers/view/msg.h"

#define CONFIG_VIRTUAL_MOUSE
/*
ALT+?
方向键，小移动

小键盘：
78946123大移动
5鼠标左键
enter鼠标右键
.(del)鼠标中键
+滚轮下
-滚轮上
*/
#define VIR_MOUSE_MOVE_BIG  50
#define VIR_MOUSE_MOVE_SMALL  12

/* 特殊按键：
ALT+TAB 切换窗口 */

static view_keyboard_t view_keyboard;

static int keyboard_process_special_key(int code, int press)
{
    if (code == KEY_E || code == KEY_e) {
        /* alt + tab */
        if (view_keyboard.key_modify & VIEW_KMOD_ALT_L) {
            if (press) {
                /* switch window */
                printf("[keyboard] [alt + tab] switch window.\n");
            }
            return 1;
        }
    }
    if (view_keyboard.virtual_mouse) {
        /* alt + ? */
        if (view_keyboard.key_modify & VIEW_KMOD_ALT) {
            if (press) {
                switch (code)
                {
                case KEY_LEFT:
                    view_mouse.x -= VIR_MOUSE_MOVE_SMALL;
                    view_mouse.motion();
                    break;
                case KEY_RIGHT:
                    view_mouse.x += VIR_MOUSE_MOVE_SMALL;
                    view_mouse.motion();
                    break;
                case KEY_UP:
                    view_mouse.y -= VIR_MOUSE_MOVE_SMALL;
                    view_mouse.motion();
                    break;
                case KEY_DOWN:
                    view_mouse.y += VIR_MOUSE_MOVE_SMALL;
                    view_mouse.motion();
                    break;
                case KEY_KP4:
                    view_mouse.x -= VIR_MOUSE_MOVE_BIG;
                    view_mouse.motion();
                    break;
                case KEY_KP6:
                    view_mouse.x += VIR_MOUSE_MOVE_BIG;
                    view_mouse.motion();
                    break;
                case KEY_KP8:
                    view_mouse.y -= VIR_MOUSE_MOVE_BIG;
                    view_mouse.motion();
                    break;
                case KEY_KP2:
                    view_mouse.y += VIR_MOUSE_MOVE_BIG;
                    view_mouse.motion();
                    break;
                case KEY_KP7:
                    view_mouse.x -= VIR_MOUSE_MOVE_BIG;
                    view_mouse.y -= VIR_MOUSE_MOVE_BIG;
                    view_mouse.motion();
                    break;
                case KEY_KP9:
                    view_mouse.x += VIR_MOUSE_MOVE_BIG;
                    view_mouse.y -= VIR_MOUSE_MOVE_BIG;
                    view_mouse.motion();
                    break;
                case KEY_KP1:
                    view_mouse.x -= VIR_MOUSE_MOVE_BIG;
                    view_mouse.y += VIR_MOUSE_MOVE_BIG;
                    view_mouse.motion();
                    break;
                case KEY_KP3:
                    view_mouse.x += VIR_MOUSE_MOVE_BIG;
                    view_mouse.y += VIR_MOUSE_MOVE_BIG;
                    view_mouse.motion();
                    break;
                case KEY_KP5:   /* left */
                    view_mouse.button_down(0);
                    break;
                case KEY_KP_ENTER:   /* right */
                case KEY_ENTER:   /* right */
                    view_mouse.button_down(2);
                    break;
                case KEY_KP_PERIOD:   /* middle */
                    view_mouse.button_down(1);
                    break;
                case KEY_KP_PLUS:   /* wheel down */
                    view_mouse.wheel(1);
                    break;
                case KEY_KP_MINUS:   /* whell up */
                    view_mouse.wheel(0);
                    break;
                default:
                    return 0;
                }
                return 1;
            } else {
                switch (code)
                {
                case KEY_KP5:   /* left */
                    view_mouse.button_up(0);
                    break;
                case KEY_KP_ENTER:   /* right */
                case KEY_ENTER:   /* right */
                    view_mouse.button_up(2);
                    break;
                case KEY_KP_PERIOD:   /* middle */
                    view_mouse.button_up(1);
                    break;
                default:
                    return 0;
                }
                return 1;
            }
        }
    }
    return 0;
}


static int keyboard_key_down(int code)
{
    // keprint("keyboard key: %d down\n", code);
    /* 处理修饰按键 */
    if (code == KEY_NUMLOCK) {
        /* 如果数字锁按键已经置位，那么就清除 */
        if (view_keyboard.key_modify & VIEW_KMOD_NUM) {
            view_keyboard.key_modify &= ~VIEW_KMOD_NUM;
        } else {    /* 没有则添加数字锁 */
            view_keyboard.key_modify |= VIEW_KMOD_NUM;    
        }
    }
    if (code == KEY_CAPSLOCK) {
        /* 如果大写锁按键已经置位，那么就清除 */
        if (view_keyboard.key_modify & VIEW_KMOD_CAPS) {
            view_keyboard.key_modify &= ~VIEW_KMOD_CAPS;
        } else {    /* 没有则添加数字锁 */
            view_keyboard.key_modify |= VIEW_KMOD_CAPS;    
        }
    }
    
    /* 处理CTRL, ALT, SHIFT*/
    switch (code) {
    case KEY_LSHIFT:    /* left shift */
        view_keyboard.key_modify |= VIEW_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        view_keyboard.key_modify |= VIEW_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        view_keyboard.key_modify |= VIEW_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        view_keyboard.key_modify |= VIEW_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        view_keyboard.key_modify |= VIEW_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        view_keyboard.key_modify |= VIEW_KMOD_CTRL_R;
        break;
    default:
        break;
    }
    
    /* 如果是一些特殊按键，就做预处理 */
    if (keyboard_process_special_key(code, 1))
        return -1;

    view_msg_t msg;
    view_msg_header(&msg, VIEW_MSG_KEY_DOWN, VIEW_TARGET_NONE);
    view_msg_data(&msg, code, view_keyboard.key_modify, 0, 0);
    view_put_global_msg(&msg);
    return 0;
}

static int keyboard_key_up(int code)
{
    // keprint("keyboard key: %d up\n", code);
    /* 处理CTRL, ALT, SHIFT*/
    switch (code) {
    case KEY_LSHIFT:    /* left shift */
        view_keyboard.key_modify &= ~VIEW_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        view_keyboard.key_modify &= ~VIEW_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        view_keyboard.key_modify &= ~VIEW_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        view_keyboard.key_modify &= ~VIEW_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        view_keyboard.key_modify &= ~VIEW_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        view_keyboard.key_modify &= ~VIEW_KMOD_CTRL_R;
        break;
    default:
        break;
    }

    /* 如果是一些特殊按键，就做预处理 */
    if (keyboard_process_special_key(code, 0))
        return -1;

    view_msg_t msg;
    view_msg_header(&msg, VIEW_MSG_KEY_UP, VIEW_TARGET_NONE);
    view_msg_data(&msg, code, view_keyboard.key_modify, 0, 0);
    view_put_global_msg(&msg);
    return 0;
}

int view_keyboard_init()
{
    view_keyboard.ledstate = 0;
    view_keyboard.handle = -1;
    view_keyboard.key_modify = 0;
    if (view_keyboard_open(&view_keyboard) < 0) {
        return -1;
    }
    view_keyboard.key_down = keyboard_key_down;
    view_keyboard.key_up = keyboard_key_up;
    view_keyboard.virtual_mouse = 1;
    return 0;
}

int view_keyboard_exit()
{
    view_keyboard.key_down = NULL;
    view_keyboard.key_up = NULL;
    view_keyboard_close(&view_keyboard);
    return 0;
}

int view_keyboard_poll()
{
    return view_keyboard_read(&view_keyboard);
}