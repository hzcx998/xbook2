#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/input.h>
#include "xgui_hal.h"

static xgui_keyboard_t xgui_keyboard;

static int keyboard_key_down(int code)
{
    printf("keyboard key: %d down\n", code);
    if (code == KEY_Q || code == KEY_q) {
        xgui_view_move_to_bottom(xgui_view_get_top());
    } else if (code == KEY_E || code == KEY_e) {
        xgui_view_move_to_top(xgui_view_get_bottom());
    } else if (code == KEY_R || code == KEY_r) {
        xgui_view_move_under_top(xgui_view_get_bottom());
    } else if (code == KEY_W || code == KEY_w) {
        xgui_view_hide(xgui_view_get_top());
    }

    return 0;
}

static int keyboard_key_up(int code)
{
    printf("keyboard key: %d up\n", code);
    return 0;
}

int xgui_keyboard_init()
{
    if (xgui_keyboard_open(&xgui_keyboard) < 0) {
        return -1;
    }
    xgui_keyboard.key_down = keyboard_key_down;
    xgui_keyboard.key_up = keyboard_key_up;
    // 将tty和键盘分离，从此tty不再接收键盘输入，使得xgui可以使用键盘
    ioctl(STDIN_FILENO, TTYIO_DETACH, 0);
    return 0;
}

int xgui_keyboard_poll()
{
    return xgui_keyboard_read(&xgui_keyboard);
}