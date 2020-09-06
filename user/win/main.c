#include <stdio.h>
#include <gapi.h>
#include <unistd.h>

int win_proc(g_msg_t *msg);

int rate_progress = 0;
int fps = 0;
#define RATE_BAR_LEN    360

void timer_handler(int layer, uint32_t msgid, uint32_t tmrid, uint32_t time)
{

    g_set_timer(layer, tmrid, 1000, timer_handler);
    
    printf("[win]: fps=%d!\n", fps);
    fps = 0;
    
}

g_bitmap_t *screen_bitmap;
int main(int argc, char *argv[]) 
{
    /* 初始化 */
    if (g_init() < 0)
        return -1;

    int win = g_new_window("win", 400, 200, 360, 240, 0);
    if (win < 0)
        return -1;
        
    g_enable_window_resize(win);
    g_set_window_minresize(win, 200, 100);
    /* 设置窗口界面 */
    
    g_show_window(win);

    /* 注册消息回调函数 */
    g_set_msg_routine(win_proc);
    
    g_set_timer(win, 0x1000, 1000, timer_handler);

    g_msg_t msg;
    while (1)
    {
        /* 获取消息，一般消息返回0，退出消息返回-1 */
        if (g_try_get_msg(&msg) < 0)
            continue;
        
        if (g_is_quit_msg(&msg))
            break;
        /* 有外部消息则处理消息 */
        g_dispatch_msg(&msg);
    }

    g_quit();

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
        //printf("[win]: mouse %d, %d\n", x, y);
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
        //g_window_rect_fill(win, x, y, w, h, color);
        #if 1
        screen_bitmap = g_new_bitmap(w, h);
        if (screen_bitmap == NULL)
            break;
        if (fps % 10 == 0 || fps == 0)
            printf("size: %d %d\n", w, h);
        g_rectfill(screen_bitmap, 0, 0, w, h, color);
        g_window_paint(win, 0, 0, screen_bitmap);
        //g_refresh_window_rect(win, 0, 0, w, h);
        
        g_del_bitmap(screen_bitmap);
        screen_bitmap = NULL;
        #endif
        //g_window_rect_fill(win,x, y, w, h, color);

        /* 图形绘制更导致绘图变慢 */
        //g_refresh_window_rect(win, x, y, w, h);
        
        g_invalid_window(win);
        g_update_window(win);
        color += 0x05040302;
        fps++;
        break;  
    

    default:
        break;
    }
    return 0;
}
