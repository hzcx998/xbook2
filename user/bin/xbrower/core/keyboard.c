#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/input.h>
#include "xbrower_hal.h"

#include "lv_drivers/indev/keyboard.h"

static xbrower_keyboard_t xbrower_keyboard;

static int keyboard_key_down(int code)
{
    //printf("keyboard key: %d down\n", code);
    xbrower_msg_t msg;
    xbrower_msg_set(&msg, XGUI_MSG_KEY_DOWN, code, 0, 0, 0);
    lv_keyboard_handler(&msg);
    return 0;
}

static int keyboard_key_up(int code)
{
    //printf("keyboard key: %d up\n", code);
    xbrower_msg_t msg;
    xbrower_msg_set(&msg, XGUI_MSG_KEY_UP, code, 0, 0, 0);
    lv_keyboard_handler(&msg);
    return 0;
}

int xbrower_keyboard_init()
{
    if (xbrower_keyboard_open(&xbrower_keyboard) < 0) {
        return -1;
    }
    xbrower_keyboard.key_down = keyboard_key_down;
    xbrower_keyboard.key_up = keyboard_key_up;
    return 0;
}

int xbrower_keyboard_exit()
{
    xbrower_keyboard_close(&xbrower_keyboard);
    return 0;
}

int xbrower_keyboard_poll()
{
    return xbrower_keyboard_read(&xbrower_keyboard);
}