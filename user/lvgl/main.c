#include <stdio.h>
#include <gapi.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include  "lvgl/lvgl.h"
#include  "lvgl/lv_examples.h"

int lv_win;
g_bitmap_t *render;
bool lv_win_exit_flag = false;
static bool mouse_pressed;
static int mouse_x, mouse_y;

#define LV_TIMER_MS 40
#define GW_DALAY_FLUSH

void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    int32_t x, y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            g_putpixel(render, x, y, *(g_color_t *) color_p);
            //g_window_put_point(lv_win, x, y, *(g_color_t *) color_p);
            //set_pixel(x, y, *color_p);  /* Put a pixel to the display.*/
            color_p++;
        }
    }
    /* 更新区域 */
    #ifdef GW_DALAY_FLUSH
    g_invalid_window(lv_win);
    g_update_window(lv_win);
    #else
    g_paint_window(lv_win, 0, 0, render);
    #endif
    //g_refresh_window_rect(lv_win, area->x1, area->y1, area->x2 - area->x1, area->y2 - area->y1);
    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
}

bool my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    data->state = mouse_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    data->point.x = mouse_x;
    data->point.y = mouse_y;
    return false;
}

static void msg_handler(struct _lv_task_t *param)
{
    (void)param;

    g_msg_t msg;
    
    /* 获取消息，一般消息返回0，退出消息返回-1 */
    if (g_try_get_msg(&msg) < 0)
        return;
    
    if (g_is_quit_msg(&msg))
        lv_win_exit_flag = true;

    /* 有外部消息则处理消息 */
    g_dispatch_msg(&msg);

}

int win_proc(g_msg_t *msg)
{
    int x, y;
    uint32_t w, h;
    switch (g_msg_get_type(msg))
    {
    case GM_MOUSE_LBTN_DOWN:
    case GM_MOUSE_LBTN_UP:
        mouse_pressed = (g_msg_get_type(msg) == GM_MOUSE_LBTN_DOWN);
    case GM_MOUSE_MOTION:
        mouse_x = g_msg_get_mouse_x(msg);
        mouse_y = g_msg_get_mouse_y(msg);
        break;
    case GM_PAINT:
        g_get_invalid(lv_win, &x, &y, &w, &h);
        #ifdef GW_DALAY_FLUSH
        /* 刷新 */
        g_paint_window(lv_win, 0, 0, render);
        #endif
        break;
    case GM_TIMER:
        lv_tick_inc(LV_TIMER_MS + 10);
        g_set_timer(lv_win, 1, LV_TIMER_MS, NULL);
        break;
    default:
        break;
    }
    return 0;
}

static void do_register(void)
{

    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * LV_VER_RES_MAX / 10];                     /*Declare a buffer for 1/10 screen size*/
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX / 10);    /*Initialize the display buffer*/    
    lv_disp_drv_t disp_drv;               /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
    disp_drv.flush_cb = my_disp_flush;    /*Set your driver function*/
    disp_drv.buffer = &disp_buf;          /*Assign the buffer to the display*/
    lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/
    
    lv_indev_drv_t indev_drv;               /*Descriptor of a input driver*/
    lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

}

int main(int argc, char **argv)
{
    
    /* 初始化 */
    if (g_init() < 0)
        return -1;
    lv_win = g_new_window("lvgl", 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX, GW_SHOW);
    if (lv_win < 0)
        return -1;
    render = g_new_bitmap(LV_HOR_RES_MAX, LV_VER_RES_MAX);
    if (render == NULL)
        return -1;
    
    /* 注册消息回调函数 */
    g_set_msg_routine(win_proc);
    
    g_set_timer(lv_win, 1, LV_TIMER_MS, NULL);

    lv_init();
    lv_win_exit_flag = false;
    lv_task_create(msg_handler, 0, LV_TASK_PRIO_HIGHEST, NULL);

    do_register();

    #if LV_USE_DEMO_WIDGETS
    lv_demo_widgets();
    #endif
    #if LV_USE_DEMO_PRINTER
    lv_demo_printer();
    #endif
    
    while (1) {
        lv_task_handler();
        if (lv_win_exit_flag)
            break;
        usleep(5000);
    }
    printf("lvgl exit!\n");
    g_quit();
    return 0;
}