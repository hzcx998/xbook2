#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/screen.h>
#include <layer/draw.h>
#include <environment/mouse.h>

#include <layer/layer.h>
#include <window/window.h>
#include <widget/widget.h>

#include <window/event.h>
#include <sys/input.h>

#include <environment/winctl.h>
#include <environment/statusbar.h>

#define DEBUG_LOCAL 0

input_mouse_t input_mouse = {0};

extern list_t layer_show_list_head;

static void __button_down(int btn)
{
    /* 如果抓住窗口后，就不能判断点击 */
    if (input_mouse.hold_window)
        return;

    gui_window_t *window;
    layer_t *layer;
    int local_mx, local_my;
    char need_update_winctl = 0;
    /* 查看点击的位置，看是否是一个 */
    list_for_each_owner_reverse (layer, &layer_show_list_head, list) {
        if (layer == layer_topest)
            continue;
        if (layer->extension == NULL)
            continue;
        
        window = (gui_window_t *) layer->extension;

        local_mx = input_mouse.x - window->x;
        local_my = input_mouse.y - window->y;
        if (local_mx >= 0 && local_mx < layer->width && 
            local_my >= 0 && local_my < layer->height) {
            
            /* 如果成功处理了控件事件，就退出 */
            if (gui_widget_mouse_button_down(&layer->widget_list_head, btn,
                local_mx, local_my) == GUI_WIDGET_EVENT_HANDLED)
                break;
            
#if DEBUG_LOCAL == 1    
            printf("touch window: pos=%d-%d, size=%d-%d\n", 
                window->x, window->y, window->width, window->height);
            printf("win: pos:%d,%d size:%d,%d mouse:%d,%d local:%d,%d\n", 
                window->x, window->y, window->width, window->height,
                    input_mouse.x, input_mouse.y, local_mx, local_my);

#endif                  
            
            if (!(window->attr & GUIW_NO_TITLE)) {
                 
                /* 有标题才判断标题栏 */
                if (ENV_IN_BOX(window->title_box, local_mx, local_my)) { /* in title */
#if DEBUG_LOCAL == 1    
                    printf("in window title\n");
#endif
                    /* 固定窗口是不能移动的 */
                    if (window->attr & GUIW_FIXED) {
                        /* 如果不是活动窗口，就切换成为活动窗口 */
                        gui_window_switch(window);
                        return;
                    }

                    switch (btn)
                    {
                    case 0: /* 鼠标左键 */
                        /* 判断是否为控制按钮，如果是，就处理，不是就可能要移动窗口，切换窗口 */
                        input_mouse.local_x = local_mx;
                        input_mouse.local_y = local_my;
                        
                        input_mouse.hold_window = window;

                        /* 如果不是活动窗口，就切换成为活动窗口 */
                        gui_window_switch(window);

                        mouse_enter_state(input_mouse);
                        break;
                    case 1: /* 鼠标右键 */

                        break;
                    default:
                        break;
                    }
                } else if (ENV_IN_BOX(window->body_box, local_mx, local_my)) {    /* in body */
                    /* 发送消息给窗口，如果不是激活窗口，就先激活 */
#if DEBUG_LOCAL == 1    
                    printf("in window body\n");
#endif
                    gui_window_switch(window);

                    /* 发送消息到窗口 */
                    gui_event_t event;
                    event.type = SGI_MOUSE_BUTTON;
                    event.button.state = SGI_PRESSED;
                    event.button.button = btn;
                    event.button.x = local_mx - window->x_off;
                    event.button.y = local_my - window->y_off;
                    gui_window_send_event(window, &event);
                }
            } else {    /* 没有标题，就发送消息，聚焦窗口 */
#if DEBUG_LOCAL == 1    
                printf("no window title\n");
#endif
                /* 记录是否需要更新窗口显示 */
                if (window != current_window)
                    need_update_winctl = 1;
                /* 聚焦窗口 */
                gui_window_focus(window);
                if (need_update_winctl)
                    gui_winctl_show();

                /* 发送事件到窗口 */
                gui_event_t event;
                event.type = SGI_MOUSE_BUTTON;
                event.button.state = SGI_PRESSED;
                event.button.button = btn;
                event.button.x = local_mx;
                event.button.y = local_my;
                gui_window_send_event(window, &event);
            }
            break;
        }
    }
}

static void __button_up(int btn)
{
#if DEBUG_LOCAL == 1    
    printf("[mouse ] %d button up.\n", btn);
    printf("[mouse ] x:%d, y:%d\n", input_mouse.x, input_mouse.y);
#endif    
    /* 处于抓住窗口，所以就释放窗口 */
    if (input_mouse.hold_window) {
        /* 更新窗口的位置 */
        input_mouse.hold_window->x = input_mouse.hold_window->layer->x;
        input_mouse.hold_window->y = input_mouse.hold_window->layer->y;

        input_mouse.hold_window = NULL;

        mouse_enter_state(input_mouse);
        input_mouse_set_state(MOUSE_CURSOR_NORMAL);
    } else {    /* 没有抓住窗口，简单传输按键弹起消息 */
        gui_window_t *window;
        layer_t *layer;
        int local_mx, local_my;
        /* 查看点击的位置，看是否是一个 */
        list_for_each_owner_reverse (layer, &layer_show_list_head, list) {
            if (layer == layer_topest)
                continue;
            if (layer->extension == NULL)
                continue;
            
            window = (gui_window_t *) layer->extension;
            local_mx = input_mouse.x - window->x;
            local_my = input_mouse.y - window->y;
            if (local_mx >= 0 && local_mx < layer->width && 
                local_my >= 0 && local_my < layer->height) {
                
                /* 如果成功处理了控件事件，就退出 */
                if (gui_widget_mouse_button_up(&layer->widget_list_head, btn,
                    local_mx, local_my) == GUI_WIDGET_EVENT_HANDLED)
                    break;
                    
                if (!(window->attr & GUIW_NO_TITLE)) {
                    /* 有标题才判断标题栏 */
                    if (ENV_IN_BOX(window->title_box, local_mx, local_my)) { /* in title */
#if DEBUG_LOCAL == 1    
                        printf("in window title\n");
#endif
                        switch (btn)
                        {
                        case 0: /* 鼠标左键 */
                            
                            break;
                        case 1: /* 鼠标右键 */

                            break;
                        default:
                            break;
                        }
                    } else if (ENV_IN_BOX(window->body_box, local_mx, local_my)) {    /* in body */
                        /* 发送消息给窗口，如果不是激活窗口，就先激活 */
#if DEBUG_LOCAL == 1    
                        printf("in window body\n");
#endif
                        gui_window_switch(window);

                        /* 发送消息到窗口 */
                        gui_event_t event;
                        event.type = SGI_MOUSE_BUTTON;
                        event.button.state = SGI_RELEASED;
                        event.button.button = btn;
                        event.button.x = local_mx - window->x_off;
                        event.button.y = local_my - window->y_off;
                        gui_window_send_event(window, &event);
                        
                    }
                } else {    /* 没有标题，就发送消息，聚焦窗口 */
                    /* 发送消息到窗口 */
                    gui_event_t event;
                    event.type = SGI_MOUSE_BUTTON;
                    event.button.state = SGI_RELEASED;
                    event.button.button = btn;    // btn
                    event.button.x = local_mx;
                    event.button.y = local_my;
                    gui_window_send_event(window, &event);
                }
                break;
            }
        }
    }
}

void __motion()
{
     /* 对鼠标进行修复 */
    if (input_mouse.x < 0)
        input_mouse.x = 0;
    if (input_mouse.y < 0)
        input_mouse.y = 0;
    if (input_mouse.x >= drv_screen.width)
        input_mouse.x = drv_screen.width - 1;
    if (input_mouse.y >= drv_screen.height)
        input_mouse.y = drv_screen.height - 1;
    
    /* 设定鼠标移动消息 */
    //printf("[mouse ] x:%d, y:%d\n", input_mouse.x, input_mouse.y);
    /* 尝试移动鼠标 */
    if (layer_topest) {
        layer_set_xy(layer_topest, input_mouse.x, input_mouse.y);
        if (input_mouse.hold_window) {    /* 让抓住的窗口跟住鼠标移动 */
            
            input_mouse_set_state(MOUSE_CURSOR_HOLD);
            int wx = input_mouse.x - input_mouse.local_x;
            int wy = input_mouse.y - input_mouse.local_y;
            
            /* 修复窗口位置 */
            if (wx < GUI_WINCTL_WIDTH)
                wx = GUI_WINCTL_WIDTH;
            if (wy < GUI_STATUSBAR_HEIGHT)
                wy = GUI_STATUSBAR_HEIGHT;

            layer_set_xy(input_mouse.hold_window->layer, wx, wy);
        } else {
            gui_window_t *window;
            layer_t *layer;
            int local_mx, local_my;
            /* 查看点击的位置，看是否是一个 */
            list_for_each_owner_reverse (layer, &layer_show_list_head, list) {
                if (layer == layer_topest)
                    continue;
                if (layer->extension == NULL)
                    continue;
                
                window = (gui_window_t *) layer->extension;
                local_mx = input_mouse.x - window->x;
                local_my = input_mouse.y - window->y;
                
                if (local_mx >= 0 && local_mx < layer->width && 
                    local_my >= 0 && local_my < layer->height) {
                    gui_widget_mouse_motion(&layer->widget_list_head, local_mx, local_my);
                
                    /* 进入某个窗口 */
                    if (input_mouse.hover_window != window && input_mouse.hover_window) { /* 从其他窗口进入当前窗口 */
                        /* 离开上个窗口的控件 */
                        gui_widget_mouse_motion(&input_mouse.hover_window->layer->widget_list_head, -1, -1);

                        /* 对于window来说，就是进入*/
                        gui_event_t event;
                        event.type = SGI_MOUSE_MOTION;
                        event.motion.state = SGI_ENTER;
                        event.motion.x = local_mx;
                        event.motion.y = local_my;
                        gui_window_send_event(window, &event);

                        /* 对于hover来说，就是离开 */
                        event.motion.state = SGI_LEAVE;
                        gui_window_send_event(input_mouse.hover_window, &event);
                    }
                    input_mouse.hover_window = window;    /* 悬停在某个窗口 */
                    
                    if (!(window->attr & GUIW_NO_TITLE)) {
                        /* 没有在标题栏才发送移动消息 */
                        if (ENV_IN_BOX(window->body_box, local_mx, local_my)) {
                            /* 发送消息到窗口 */
                            gui_event_t event;
                            event.type = SGI_MOUSE_MOTION;
                            event.motion.state = 0;
                            event.motion.x = local_mx - window->x_off;
                            event.motion.y = local_my - window->y_off;
                            gui_window_send_event(window, &event);
                        }
                    } else {
                        /* 发送消息到窗口 */
                        gui_event_t event;
                        event.type = SGI_MOUSE_MOTION;
                        event.motion.state = 0;
                        event.motion.x = local_mx;
                        event.motion.y = local_my;
                        gui_window_send_event(window, &event);
                    }
   
                    break;
                }
            }
        }
    }
}

/* 需要对齐，不然数据就会出现混乱 */
char __attribute__((aligned(32)))__mouse_cursor_normal[MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT] = {
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,2,1,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,2,1,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
/* 需要对齐，不然数据就会出现混乱 */
char __attribute__((aligned(32)))__mouse_cursor_hold[MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT] = {
    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,1,2,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,1,2,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,2,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,0,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

/**
 * input_mouse_draw_buffer - 往缓冲区里面绘制鼠标
 */
static void input_mouse_draw_buffer(GUI_COLOR *buffer, char *data)
{
    int x, y;
	for (y = 0; y < MOUSE_CURSOR_HEIGHT; y++) {
		for (x = 0; x < MOUSE_CURSOR_WIDTH; x++) {
			if (data[y * MOUSE_CURSOR_WIDTH + x] == 0) {
                /* 透明色 */
                buffer[y * MOUSE_CURSOR_WIDTH + x] = COLOR_ARGB(0, 0, 0, 0);
            } else if (data[y * MOUSE_CURSOR_WIDTH + x] == 1) {
				/* 边框色 */
                buffer[y * MOUSE_CURSOR_WIDTH + x] = COLOR_WHITE;
            } else if (data[y * MOUSE_CURSOR_WIDTH + x] == 2) {
                /* 填充色 */
                buffer[y * MOUSE_CURSOR_WIDTH + x] = COLOR_BLACK;
            }
		}
	}
}

/**
 * 根据鼠标的状态绘制鼠标
 * 
 */
void input_mouse_draw(mouse_cursor_state_t state)
{
    switch (state)
    {
    case MOUSE_CURSOR_NORMAL:
        input_mouse_draw_buffer(input_mouse.layer->buffer, __mouse_cursor_normal);
        break;
    case MOUSE_CURSOR_HOLD:
        input_mouse_draw_buffer(input_mouse.layer->buffer, __mouse_cursor_hold);
        
        break;
    
    default:
        break;
    }
    /* 刷新鼠标图层 */
    layer_refresh(input_mouse.layer, 0, 0, input_mouse.layer->width, input_mouse.layer->height);
}

void input_mouse_set_state(mouse_cursor_state_t state)
{
    if (input_mouse.state_changed == false) {
        //printf("[mouse] change state %d to %d\n", input_mouse.state, state);
        input_mouse_draw(state);
        input_mouse.state_changed = true;
    }
}

int init_mouse_input()
{
    input_mouse.state = MOUSE_CURSOR_NORMAL;
    input_mouse.state_changed = false;
    input_mouse.x = drv_screen.width / 2;
    input_mouse.y = drv_screen.height / 2;
    input_mouse.local_x = 0;
    input_mouse.local_y = 0;
    input_mouse.hold_window = NULL;
    input_mouse.hover_window = NULL;
    input_mouse.button_down   = __button_down;
    input_mouse.button_up     = __button_up;
    input_mouse.motion     = __motion;
    
    /* 顶层图层是鼠标图层 */
    layer_topest = create_layer(MOUSE_CURSOR_WIDTH, MOUSE_CURSOR_HEIGHT);
    if (layer_topest == NULL) {
        printf("create layer failed!\n");
        return -1;
    }
    input_mouse.layer = layer_topest;

#if DEBUG_LOCAL == 1 
    printf("create layer: addr %x, buffer %x, width %d, height %d\n",
        layer_topest, layer_topest->buffer, layer_topest->width, layer_topest->height);
#endif

    /* 绘制鼠标图层 */
    input_mouse_draw_buffer(input_mouse.layer->buffer, __mouse_cursor_normal);
    
    /* 设置鼠标图层高度 */
    layer_set_z(layer_topest, 0);
    
    /* 设置鼠标图层位置 */
    layer_set_xy(layer_topest, input_mouse.x, input_mouse.y);

    return 0;
}
