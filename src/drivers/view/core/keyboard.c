#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/input.h>
#include "drivers/view/hal.h"
#include "drivers/view/msg.h"

static view_keyboard_t view_keyboard;

static int keyboard_key_down(int code)
{
    keprint("keyboard key: %d down\n", code);
    view_msg_t msg;
    view_msg_header(&msg, VIEW_MSG_KEY_DOWN, VIEW_TARGET_NONE);
    view_msg_data(&msg, code, 0, 0, 0);
    view_put_global_msg(&msg);
    return 0;
}

static int keyboard_key_up(int code)
{
    keprint("keyboard key: %d up\n", code);
    view_msg_t msg;
    view_msg_header(&msg, VIEW_MSG_KEY_UP, VIEW_TARGET_NONE);
    view_msg_data(&msg, code, 0, 0, 0);
    view_put_global_msg(&msg);
    return 0;
}

int view_keyboard_init()
{
    if (view_keyboard_open(&view_keyboard) < 0) {
        return -1;
    }
    view_keyboard.key_down = keyboard_key_down;
    view_keyboard.key_up = keyboard_key_up;
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