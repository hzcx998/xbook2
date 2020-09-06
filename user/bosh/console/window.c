#include <gapi.h>
#include <stdio.h>

#include <sh_window.h>
#include <sh_console.h>
#include <sh_clipboard.h>
#include <sh_shell.h>

sh_window_t sh_window;

int init_window()
{
    /* 初始化 */
    if (g_init() < 0)
        return -1;

    int win = g_new_window(APP_NAME, 100, 100, SH_WIN_WIDTH, SH_WIN_HEIGHT, GW_NO_MAXIM);
    if (win < 0)
        return -1;

    sh_window.win = win;

    g_show_window(win);
    
    g_set_window_icon(win, "/res/icon/bosh.png");

    /* 注册消息回调函数 */
    g_set_msg_routine(process_window);
    
    return 0;
}

void main_window()
{
    g_msg_t msg;
    while (1)
    {
        /* 获取消息，一般消息返回0，退出消息返回-1 */
        if (g_get_msg(&msg) < 0)
            continue;
        
        if (g_is_quit_msg(&msg))
            break;
        /* 有外部消息则处理消息 */
        g_dispatch_msg(&msg);
    }
}

int exit_window()
{

    return g_quit();
}

int process_window(g_msg_t *msg)
{
    int x, y;
    uint32_t w, h;
    int keycode, keymod;
    int win;
    switch (g_msg_get_type(msg))
    {
    case GM_MOUSE_LBTN_DOWN:
        x = g_msg_get_mouse_x(msg);
        y = g_msg_get_mouse_y(msg);
        clipboard_start_select(x, y);
        break;
    case GM_MOUSE_RBTN_DOWN:
        clipboard_copy_select();
        break;
    case GM_MOUSE_LBTN_UP:
        x = g_msg_get_mouse_x(msg);
        y = g_msg_get_mouse_y(msg);
        clipboard_end_select(x, y);
        break;
    case GM_MOUSE_MOTION:
        x = g_msg_get_mouse_x(msg);
        y = g_msg_get_mouse_y(msg);
        clipboard_move_select(x, y);
        break;
    case GM_KEY_DOWN:
        g_translate_msg(msg);
        keycode = g_msg_get_key_code(msg);
        keymod = g_msg_get_key_modify(msg);
        con_get_key(keycode, keymod);
        break;
    case GM_PAINT:
        win = g_msg_get_target(msg);
        g_get_invalid(win, &x, &y, &w, &h);
        con_screen.flush();

        break; 
    default:
        break;
    }
    return 0;
}

int process_window2(g_msg_t *msg)
{
    int x, y;
    uint32_t w, h;
    int keycode, keymod;
    int win;
    switch (g_msg_get_type(msg))
    {
    case GM_KEY_DOWN:
        g_translate_msg(msg);
        keycode = g_msg_get_key_code(msg);
        keymod = g_msg_get_key_modify(msg);
        return con_xmit_key(keycode, keymod);
    case GM_PAINT:
        win = g_msg_get_target(msg);
        g_get_invalid(win, &x, &y, &w, &h);
        con_screen.flush();
        break; 
    default:
        break;
    }
    return 0;
}

int poll_window()
{
    g_msg_t msg;
    /* 尝试获取消息，没有则返回-1 */
    if (g_try_get_msg(&msg) < 0)
        return -1;
    
    if (g_is_quit_msg(&msg)) {
        /* 退出执行 */
        exit_shell();
    }
    /* 有外部消息则处理消息 */
    return g_dispatch_msg(&msg);
}

void sh_window_rect_fill(int x, int y, uint32_t width, uint32_t height, uint32_t color)
{
    #if 1
    g_bitmap_t *bmp = g_new_bitmap(width, height);
    if (!bmp)
        return;
    g_rectfill(bmp, 0, 0, width, height, color);
    g_window_paint_ex(sh_window.win, x, y, bmp);
    g_del_bitmap(bmp);

    #else
    g_window_rect_fill(sh_window.win, x, y, width, height, color);
    #endif
}

void sh_window_rect(int x, int y, uint32_t width, uint32_t height, uint32_t color)
{
    #if 1
    g_bitmap_t *bmp = g_new_bitmap(width, height);
    if (!bmp)
        return;
    g_rect(bmp, 0, 0, width, height, color);
    g_window_paint_ex(sh_window.win, x, y, bmp);
    g_del_bitmap(bmp);
    #else
    g_window_rect(sh_window.win, x, y, width, height, color);
    #endif
}

void sh_window_char(int x, int y, char ch, uint32_t color)
{
    #if 1
    g_bitmap_t *bmp = g_new_bitmap(8, 16);
    if (!bmp)
        return;
    g_char(bmp, 0, 0, ch, color);
    //g_window_paint()
    g_window_paint_ex(sh_window.win, x, y, bmp);
    g_del_bitmap(bmp);
    #else 
    g_window_char(sh_window.win, x, y, ch, color);
    #endif
}

void sh_window_update(int left, int top, int right, int bottom)
{
    g_refresh_window_region(sh_window.win, left, top, right, bottom);
}


int sh_window_size(uint32_t *w, uint32_t *h)
{
    g_window_t *gw = g_find_window(sh_window.win);
    if (gw == NULL) {
        return -1;
    }
    *w = gw->win_width;
    *h = gw->win_height;
    return 0;
}