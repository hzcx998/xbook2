#include <stdio.h>
#include <gapi.h>
#include <unistd.h>

int win_proc(g_msg_t *msg);

int rate_progress = 0;

#define RATE_BAR_LEN    360

void timer_handler(int layer, uint32_t msgid, uint32_t tmrid, uint32_t time)
{
    //printf("[win]: timer msgid=%d tmrid=%x layer=%d time=%x\n", msgid, tmrid, layer, time);
    
    int w = RATE_BAR_LEN / 100;
    w *= rate_progress;
    rate_progress += 1;
    
    if (rate_progress <= 100) {
        g_set_timer(layer, tmrid, 50, timer_handler);
    } else {
        g_del_timer(layer, tmrid);
    }
    
    /* 绘制进度条 */
    g_window_rect_fill(layer, 0, 0, RATE_BAR_LEN, 240, GC_RED);
    g_window_rect_fill(layer, 0, 0, w, 240, GC_YELLOW);
    g_refresh_window_rect(layer, 0, 0, RATE_BAR_LEN, 240);
    w = RATE_BAR_LEN / 100;
    w *= rate_progress;
    rate_progress += 1;
    g_window_rect_fill(layer, 0, 0, w, 240, GC_YELLOW);
    g_refresh_window_rect(layer, 0, 0, RATE_BAR_LEN, 240);
    
}

int main(int argc, char *argv[]) 
{
    printf("win start.\n");
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
    
    uint32_t tmrid = g_set_timer(win1, 0, 1000, NULL);
    printf("set timer id=%d.\n", tmrid);
    uint32_t tmrid1 = g_set_timer(win1, 0x1000, 50, timer_handler);
    printf("set timer id=%d.\n", tmrid1);
    
    if (!g_del_timer(win1, tmrid))
        printf("del timer id=%d ok.\n", tmrid);

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
    uint32_t w, h;
    int keycode, keymod;
    int win;
    static uint32_t color = GC_RED;

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
    case GM_PAINT:
        win = g_msg_get_target(msg);
        g_get_invalid(win, &x, &y, &w, &h);
        /*
        g_window_rect_fill(layer, 0, 0, 100, 50, GC_BLUE);
        g_window_rect_fill(layer, -50, -50, 100, 100, GC_GREEN);

        g_window_rect_fill(layer, w-100, h-100, 100, 50, GC_BLUE);
        g_window_rect_fill(layer, w-50, h-50, 100, 50, GC_GREEN);
        */
        g_refresh_window_rect(win, x, y, w, h);
        
        /*
        g_window_rect_fill(win, 0, 0, 360, 240, color);
        g_invalid_window(win);
        g_update_window(win);
        color += 0x05040302;
        */

        break;  
    case GM_TIMER:
        printf("[win1]: get timer!\n");
        
        break;  
    default:
        break;
    }
    return 0;
}
