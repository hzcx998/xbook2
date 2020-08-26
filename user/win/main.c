#include <stdio.h>
#include <gapi.h>
#include <unistd.h>

int win_proc(g_msg_t *msg);
int main(int argc, char *argv[]) {

restart: 
    /* 初始化 */
    if (g_init() < 0)
        return -1;

    int win = g_new_window("win", 400, 200, 360, 240);
    if (win < 0)
        return -1;
        
    g_enable_window_resize(win);
    g_set_window_minresize(win, 200, 100);
    /* 设置窗口界面 */
    
    g_show_window(win);

    int win1 = g_new_window("win", 200, 100, 360, 240);
    if (win1 < 0)
        return -1;
        
    //g_enable_window_resize(win1, 200, 100);
    /* 设置窗口界面 */
    
    g_show_window(win1);

    /* 注册消息回调函数 */
    g_set_msg_routine(win_proc);
    
    uint32_t color = GC_RGB(0, 0, 0);
    int i = 0;
    g_msg_t msg;
    while (1)
    {
        #if 0
        i++;
        if (i % 1000 == 0) {

        color = GC_RGB(i * 5, i * 10, i * 15);;
        /* 刷新图层 */
        g_layer_rect_fill(win, 0, 0, 360, 240, color);
        g_layer_refresh_rect(win, 0, 0, 360, 240);

        /* 刷新图层 */
        g_layer_rect_fill(win1, 0, 0, 360, 240, color);
        g_layer_refresh_rect(win1, 0, 0, 360, 240);
        
        }
        #endif
        /* 获取消息，一般消息返回0，退出消息返回-1 */
        if (g_try_get_msg(&msg) < 0)
            continue;
        
        if (g_is_quit_msg(&msg))
            break;
        /* 有外部消息则处理消息 */
        g_dispatch_msg(&msg);
    }
    g_quit();
    goto restart;
    return 0;
}

int win_proc(g_msg_t *msg)
{
    int x, y;
    int keycode, keymod;
    switch (g_msg_get_type(msg))
    {
    case GM_KEY_DOWN:
        keycode = g_msg_get_key_code(msg);
        keymod = g_msg_get_key_modify(msg);
        
        printf("[win]: down keycode=%d:%c modify=%x\n", keycode, keycode, keymod);
        g_translate_msg(msg);
        keycode = g_msg_get_key_code(msg);
        keymod = g_msg_get_key_modify(msg);
        printf("[win]: down keycode=%d:%c modify=%x\n", keycode, keycode, keymod);
        
        if (keycode == GK_SPACE) {
            printf("[win]: down space\n");
        }

        break;
    case GM_KEY_UP:
        keycode = g_msg_get_key_code(msg);
        keymod = g_msg_get_key_modify(msg);
        printf("[win]: up keycode=%d:%c modify=%x\n", keycode, keycode, keymod);
        g_translate_msg(msg);
        keycode = g_msg_get_key_code(msg);
        keymod = g_msg_get_key_modify(msg);
        printf("[win]: up keycode=%d:%c modify=%x\n", keycode, keycode, keymod);
        if (keycode == GK_SPACE) {
            printf("[win]: up space\n");
        }
        break;
    case GM_MOUSE_MOTION:
    case GM_MOUSE_LBTN_DOWN:
    case GM_MOUSE_LBTN_UP:
    case GM_MOUSE_LBTN_DBLCLK:
    case GM_MOUSE_RBTN_DOWN:
    case GM_MOUSE_RBTN_UP:
    case GM_MOUSE_RBTN_DBLCLK:
    case GM_MOUSE_MBTN_DOWN:
    case GM_MOUSE_MBTN_UP:
    case GM_MOUSE_MBTN_DBLCLK:
    case GM_MOUSE_WHEEL:
        x = g_msg_get_mouse_x(msg);
        y = g_msg_get_mouse_y(msg);
        printf("[win]: mouse %d, %d\n", x, y);
        break;
    default:
        break;
    }
    return 0;
}
