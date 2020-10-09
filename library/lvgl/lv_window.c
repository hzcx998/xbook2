#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include  "lvgl.h"
#include  "lv_drv_conf.h"
#include  "lvgl_window.h"

#define LV_WINDOW_LOOP_USLEEP_VAL  5000
#define LV_WINDOW_TIMER_MS 40

/* 窗口句柄 */
int lv_winhdl = -1;

static int lv_timer_id = 1;
static int lv_usleep_val = 0;
static bool lv_win_exit_flag = false;
static void (*lv_win_handler)(void) = NULL;

static lv_indev_t *lv_window_mouse_drv = NULL;
static lv_indev_t *lv_window_kbd_drv = NULL;
static lv_indev_t *lv_window_mousewheel_drv = NULL;

lv_indev_t *lv_window_get_kbd()
{
    return lv_window_kbd_drv;
}

lv_indev_t *lv_window_get_mouse()
{
    return lv_window_mouse_drv;
}

lv_indev_t *lv_window_get_mousewheel()
{
    return lv_window_mousewheel_drv;
}

static void do_msg_handle(struct _lv_task_t *param)
{
    (void)param;

    g_msg_t msg;
    if (g_try_get_msg(&msg) < 0)
        return;
    if (g_is_quit_msg(&msg))
        lv_win_exit_flag = true;
    g_dispatch_msg(&msg);
}

static void do_win_handle(struct _lv_task_t *param)
{
    (void)param;

    if (lv_win_handler)
        lv_win_handler();
}



static int do_win_proc(g_msg_t *msg)
{
    int x, y;
    uint32_t w, h;
    switch (g_msg_get_type(msg))
    {
    case GM_MOUSE_LBTN_DOWN:
    case GM_MOUSE_LBTN_UP:
    case GM_MOUSE_MOTION:
        lv_mouse_handler(msg);
        break;
    case GM_MOUSE_MBTN_DOWN:
    case GM_MOUSE_MBTN_UP:
    case GM_MOUSE_WHEEL_UP:
    case GM_MOUSE_WHEEL_DOWN:
        lv_mousewheel_handler(msg);
        break;
    case GM_PAINT:
        g_get_invalid(lv_winhdl, &x, &y, &w, &h);
        #ifdef LV_DISPLAY_PAINT
        /* 刷新 */
        g_paint_window(lv_winhdl, 0, 0, lv_display_render);
        #endif
        break;
    case GM_TIMER:
        lv_tick_inc(LV_WINDOW_TIMER_MS + 10);
        g_set_timer(lv_winhdl, lv_timer_id, LV_WINDOW_TIMER_MS, NULL);
        break;
    case GM_KEY_DOWN:
    case GM_KEY_UP:                         /*Button release*/
        lv_keyboard_handler(msg);
        break;
    default:
        break;
    }
    
    return 0;
}

static int do_register_drv(uint32_t width, uint32_t height)
{
    /*------------------
     * Display
     * -----------------*/
    if (lv_display_init(width, height) < 0)
        return -1;
    static lv_disp_buf_t disp_buf;
    
    lv_color_t *buf = malloc((width * height * sizeof(lv_color_t)) / 10);                     /*Declare a buffer for 1/10 screen size*/
    if (buf == NULL) {
        lv_display_exit();
        return -1;
    }

    lv_disp_buf_init(&disp_buf, buf, NULL, width * height / 10);    /*Initialize the display buffer*/    
    lv_disp_drv_t disp_drv;               /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
    disp_drv.hor_res = width;
    disp_drv.ver_res = height;
    disp_drv.flush_cb = lv_display_flush;    /*Set your driver function*/
    disp_drv.buffer = &disp_buf;          /*Assign the buffer to the display*/
    lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/

    /*------------------
     * Mouse
     * -----------------*/

    /*Initialize your touchpad if you have*/
    lv_mouse_init();

    lv_indev_drv_t indev_drv;               /*Descriptor of a input driver*/
    /*Register a mouse input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lv_mouse_read;
    lv_window_mouse_drv = lv_indev_drv_register(&indev_drv);

    /*------------------
     * Keypad
     * -----------------*/
    lv_keyboard_init();
    
    /*Register a keypad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = lv_keyboard_read;
    lv_window_kbd_drv = lv_indev_drv_register(&indev_drv);

    /* Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     * add objects to the group with `lv_group_add_obj(group, obj)`
     * and assign this input device to group to navigate in it:
     * `lv_indev_set_group(indev_keypad, group);` */

    /*------------------
     * Mouse Wheel
     * -----------------*/

    /*Initialize your encoder if you have*/
    lv_mousewheel_init();

    /*Register a encoder input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = lv_mousewheel_read;
    lv_window_mousewheel_drv = lv_indev_drv_register(&indev_drv);

    /* Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     * add objects to the group with `lv_group_add_obj(group, obj)`
     * and assign this input device to group to navigate in it:
     * `lv_indev_set_group(indev_encoder, group);` */

    return 0;
}

void lv_window_exit()
{
    g_quit();
}

int lv_window_init(char *title, int x, int y, uint32_t width, uint32_t height)
{
    if (g_init() < 0)
        return -1;

    lv_winhdl = g_new_window(title, x, y, width, height, GW_SHOW);
    if (lv_winhdl < 0)
        return -1;
        
    lv_timer_id = 1;
    lv_win_exit_flag = false;
    lv_usleep_val = LV_WINDOW_LOOP_USLEEP_VAL;

    g_set_msg_routine(do_win_proc);

    lv_init();

    lv_task_create(do_msg_handle, 0, LV_TASK_PRIO_HIGHEST, NULL);
    lv_task_create(do_win_handle, 0, LV_TASK_PRIO_HIGHEST, NULL);

    if (do_register_drv(width, height) < 0) {
        g_del_window(lv_winhdl);
        g_quit();
        return -1;
    }

    // 设置退出函数
    atexit(lv_window_exit);

    g_set_timer(lv_winhdl, lv_timer_id, LV_WINDOW_TIMER_MS, NULL);
    
    return 0;
}

void lv_window_set_handler(void (*handler)(void))
{
    lv_win_handler = handler;
}

void lv_window_quit()
{
    lv_win_exit_flag = true;
}

void lv_window_loop()
{
    while (1) {
        lv_task_handler();
        if (lv_win_exit_flag)
            break;
        usleep(lv_usleep_val);
    }
}
