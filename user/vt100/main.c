#include <stdio.h>
#include <gapi.h>
#include "vt100.h"
#include "vt.h"

int g_win;

int win_proc(g_msg_t *msg);

int win_proc(g_msg_t *msg)
{
    int x, y;
    uint32_t w, h;
    int keycode;
    g_window_t *gw = g_find_window(g_msg_get_target(msg));
    if (!gw)
        return -1;
    switch (g_msg_get_type(msg))
    {
    case GM_KEY_DOWN:
        keycode = g_msg_get_key_code(msg);
        _putc(keycode);
        break;
    case GM_KEY_UP:
        keycode = g_msg_get_key_code(msg);
        break;
    case GM_PAINT:
        g_get_invalid(gw->layer, &x, &y, &w, &h);
        //test_colors();
        g_refresh_window_rect(gw->layer, x, y, w, h);
        break;  
    case GM_RESIZE:
        init_console(gw->win_width, gw->win_height);
        break;
    default:
        break;
    }
    return 0;
}

int launch_vt100()
{
    /* 初始化 */
    if (g_init() < 0)
        return -1;

    g_win = g_new_window("vt100", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (g_win < 0)
        return -1;
        
    g_enable_window_resize(g_win);
    g_set_window_minresize(g_win, 200, 100);

    init_console(SCREEN_WIDTH, SCREEN_HEIGHT);

    g_show_window(g_win);

    vt100_test(0);
    //_puts("hello, vt100!\nabc\b \e[1D123");

    int i;
    for (i = 0; i < 100; i++) {
        _puts("h");
        if (in_line_end) {
            _puts("\n");
        }
    }
    g_msg_t msg;
    while (1)
    {
        if (!g_try_get_msg(&msg))
            continue;
        if (g_is_quit_msg(&msg))
            break;
        /* 有外部消息则处理消息 */
        win_proc(&msg);
    }
    g_quit();
    return 0;
}

int main(int argc, char *argv[])
{
    printf("vt100 start!\n");
    
    return launch_vt100();
}